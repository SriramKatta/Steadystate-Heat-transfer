#include "PDE.h"
#include <math.h>
#include <iostream>
#include <omp.h>
#ifdef LIKWID_PERFMON
#include <likwid.h>
#endif
// default boundary function as in ex01
double defaultBoundary(int i, int j, double h_x, double h_y)
{
  return sin(M_PI * i * h_x) * sinh(M_PI * j * h_y);
}
// default rhs function as in ex01
double zeroFunc(int i, int j, double h_x, double h_y)
{
  return 0 + 0 * i * h_x + 0 * j * h_y;
}

// Constructor
PDE::PDE(int len_x_, int len_y_, int grids_x_, int grids_y_) : len_x(len_x_), len_y(len_y_), grids_x(grids_x_ + 2 * HALO), grids_y(grids_y_ + 2 * HALO)
{
  h_x = static_cast<double>(len_x) / (grids_x - 1.0);
  h_y = static_cast<double>(len_y) / (grids_y - 1.0);

  initFunc = zeroFunc;

  // by default all boundary is Dirichlet
#pragma omp parallel for
  for (int i = 0; i < 4; ++i)
    boundary[i] = Dirichlet;
#pragma omp parallel for
  for (int i = 0; i < 4; ++i)
    boundaryFunc[i] = zeroFunc;
}

int PDE::numGrids_x(bool halo)
{
  int halo_x = halo ? 0 : 2 * HALO;
  return (grids_x - halo_x);
}

int PDE::numGrids_y(bool halo)
{
  int halo_y = halo ? 0 : 2 * HALO;
  return (grids_y - halo_y);
}

void PDE::init(Grid *grid)
{
#ifdef DEBUG
  assert((grid->numGrids_y(true) == grids_y) && (grid->numGrids_x(true) == grids_x));
#endif
  grid->fill(std::bind(initFunc, std::placeholders::_1, std::placeholders::_2, h_x, h_y));
}

// Boundary Condition
void PDE::applyBoundary(Grid *u)
{
#ifdef DEBUG
  assert((u->numGrids_y(true) == grids_y) && (u->numGrids_x(true) == grids_x));
#endif
  if (boundary[NORTH] == Dirichlet)
  {
    u->fillBoundary(std::bind(boundaryFunc[NORTH], std::placeholders::_1, std::placeholders::_2, h_x, h_y), NORTH);
  }
  if (boundary[SOUTH] == Dirichlet)
  {
    u->fillBoundary(std::bind(boundaryFunc[SOUTH], std::placeholders::_1, std::placeholders::_2, h_x, h_y), SOUTH);
  }
  if (boundary[EAST] == Dirichlet)
  {
    u->fillBoundary(std::bind(boundaryFunc[EAST], std::placeholders::_1, std::placeholders::_2, h_x, h_y), EAST);
  }
  if (boundary[WEST] == Dirichlet)
  {
    u->fillBoundary(std::bind(boundaryFunc[WEST], std::placeholders::_1, std::placeholders::_2, h_x, h_y), WEST);
  }
}

// It refreshes Neumann boundary, 2 nd argument is to allow for refreshing with 0 shifts, ie in coarser levels
void PDE::refreshBoundary(Grid *u)
{
#ifdef DEBUG
  assert((u->numGrids_y(true) == grids_y) && (u->numGrids_x(true) == grids_x));
#endif
  if (boundary[NORTH] == Neumann)
  {
    u->copyToHalo(std::bind(boundaryFunc[NORTH], std::placeholders::_1, std::placeholders::_2, h_x, h_y), NORTH);
  }
  if (boundary[SOUTH] == Neumann)
  {
    u->copyToHalo(std::bind(boundaryFunc[SOUTH], std::placeholders::_1, std::placeholders::_2, h_x, h_y), SOUTH);
  }
  if (boundary[EAST] == Neumann)
  {
    u->copyToHalo(std::bind(boundaryFunc[EAST], std::placeholders::_1, std::placeholders::_2, h_x, h_y), EAST);
  }
  if (boundary[WEST] == Neumann)
  {
    u->copyToHalo(std::bind(boundaryFunc[WEST], std::placeholders::_1, std::placeholders::_2, h_x, h_y), WEST);
  }
}

// Applies stencil operation on to x
// i.e., lhs = A*x
void PDE::applyStencil(Grid *lhs, Grid *x)
{
  START_TIMER(APPLY_STENCIL);

#ifdef DEBUG
  assert((lhs->numGrids_y(true) == grids_y) && (lhs->numGrids_x(true) == grids_x));
  assert((x->numGrids_y(true) == grids_y) && (x->numGrids_x(true) == grids_x));
#endif
  const int xSize = numGrids_x(true);
  const int ySize = numGrids_y(true);

  const double w_x = 1.0 / (h_x * h_x);
  const double w_y = 1.0 / (h_y * h_y);
  const double w_c = 2.0 * w_x + 2.0 * w_y;

  int collimit = (1.25 * 1000 * 1000) / 48;
  int colend = 0;

  collimit = std::min(collimit, xSize - 1);

#pragma omp parallel private(colend)
  {
#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("APPLY_STENCIL");
#endif
    for (int colstart = 1; colstart < xSize - 1; colstart += collimit)
    {
      colend = std::min(colstart + collimit, xSize) - 1;
#pragma omp for nowait
      for (int j = 1; j < ySize - 1; ++j)
      {
#pragma omp simd
        for (int i = colstart; i < colend; ++i)
        {
          (*lhs)(j, i) = w_c * (*x)(j, i) - w_y * ((*x)(j + 1, i) + (*x)(j - 1, i)) - w_x * ((*x)(j, i + 1) + (*x)(j, i - 1));
        }
      }
    }

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("APPLY_STENCIL");
#endif
  }

  STOP_TIMER(APPLY_STENCIL);
}

// Applies stencil operation on to x
// i.e., lhs = A*x
double PDE::applyStencil_dot(Grid *lhs, Grid *x)
{
  START_TIMER(APPLY_STEN_DOT);

#ifdef DEBUG
  assert((lhs->numGrids_y(true) == grids_y) && (lhs->numGrids_x(true) == grids_x));
  assert((x->numGrids_y(true) == grids_y) && (x->numGrids_x(true) == grids_x));
#endif
  const int xSize = numGrids_x(true);
  const int ySize = numGrids_y(true);

  const double w_x = 1.0 / (h_x * h_x);
  const double w_y = 1.0 / (h_y * h_y);
  const double w_c = 2.0 * w_x + 2.0 * w_y;
  double safetymargin = 2.0;
  double numrowstostore = 3.0;
  double datatypesize = sizeof((*lhs)(0, 0));
  double cachesizeinbytes = 1.25e6;

  int collimit = (cachesizeinbytes) / (safetymargin * datatypesize * numrowstostore);
  int colend = 0;

  collimit = std::min(collimit, xSize - 1);
  double dotprod = 0.0;

#pragma omp parallel private(colend)
  {
#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("APPLY_STEN_DOT");
#endif
    for (int colstart = 1; colstart < xSize - 1; colstart += collimit)
    {
      colend = std::min(colstart + collimit, xSize) - 1;
#pragma omp for nowait reduction(+ : dotprod)
      for (int j = 1; j < ySize - 1; ++j)
      {
#pragma omp simd
        for (int i = colstart; i < colend; ++i)
        {
          (*lhs)(j, i) = w_c * (*x)(j, i) - w_y * ((*x)(j + 1, i) + (*x)(j - 1, i)) - w_x * ((*x)(j, i + 1) + (*x)(j, i - 1));
          dotprod += ((*lhs)(j, i) * (*x)(j, i));
        }
      }
    }

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("APPLY_STEN_DOT");
#endif
  }

  STOP_TIMER(APPLY_STEN_DOT);
  return dotprod;
}

// GS preconditioning; solving for x: A*x=rhs
void PDE::GSPreCon(Grid *rhs, Grid *x)
{
  START_TIMER(GS_PRE_CON);

#ifdef DEBUG
  assert((rhs->numGrids_y(true) == grids_y) && (rhs->numGrids_x(true) == grids_x));
  assert((x->numGrids_y(true) == grids_y) && (x->numGrids_x(true) == grids_x));
#endif
  const int xSize = x->numGrids_x(true);
  const int ySize = x->numGrids_y(true);

  const double w_x = 1.0 / (h_x * h_x);
  const double w_y = 1.0 / (h_y * h_y);
  const double w_c = 1.0 / static_cast<double>((2.0 * w_x + 2.0 * w_y));

  int num_th, th_id, jj, j, i;
#pragma omp parallel private(num_th, th_id, jj, i, j)
  {

    num_th = omp_get_num_threads();
    th_id = omp_get_thread_num();

    int interval = (xSize - 2) / num_th;
    int interval_s = interval * th_id + 1;
    int interval_e = (th_id == (num_th - 1)) ? (xSize - 2) : (interval_s + interval - 1);
#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("GS_PRE_CON_FW");
#endif
    // forward substitution
    for (j = 1; j < ySize - 1 + num_th - 1; ++j)
    {
      jj = j - th_id;
      if (jj >= 1 && jj < ySize - 1)
      {
        // #pragma omp simd
        for (i = interval_s; i <= interval_e; ++i)
        {
          (*x)(jj, i) = w_c * ((*rhs)(jj, i) + (w_y * (*x)(jj - 1, i) + w_x * (*x)(jj, i - 1)));
        }
      }
#pragma omp barrier
    }
#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("GS_PRE_CON_FW");
#endif

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("GS_PRE_CON_BW");
#endif
    // backward substitution
    for (j = ySize - 2 + num_th - 1; j > 0; --j)
    {
      jj = j - th_id;
      if (jj < ySize - 1 && jj >= 1)
      {
        // #pragma omp simd
        for (i = interval_e; i >= interval_s; --i)
        {
          (*x)(jj, i) = (*x)(jj, i) + w_c * (w_y * (*x)(jj + 1, i) + w_x * (*x)(jj, i + 1));
        }
      }
#pragma omp barrier
    }

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("GS_PRE_CON_BW");
#endif
  }

  STOP_TIMER(GS_PRE_CON);
}

// GS preconditioning; solving for x: A*x=rhs
double PDE::GSPreCon_dot(Grid *rhs, Grid *x)
{
  START_TIMER(GS_PRE_CON_DOT);

#ifdef DEBUG
  assert((rhs->numGrids_y(true) == grids_y) && (rhs->numGrids_x(true) == grids_x));
  assert((x->numGrids_y(true) == grids_y) && (x->numGrids_x(true) == grids_x));
#endif
  const int xSize = x->numGrids_x(true);
  const int ySize = x->numGrids_y(true);

  const double w_x = 1.0 / (h_x * h_x);
  const double w_y = 1.0 / (h_y * h_y);
  const double w_c = 1.0 / static_cast<double>((2.0 * w_x + 2.0 * w_y));
  double dotprod = 0.0;

  int num_th, th_id, jj, j, i;
#pragma omp parallel private(num_th, th_id, jj, i, j) reduction(+ : dotprod)
  {
    num_th = omp_get_num_threads();
    th_id = omp_get_thread_num();

    int interval = (xSize - 2) / num_th;
    int interval_s = interval * th_id + 1;
    int interval_e = (th_id == (num_th - 1)) ? (xSize - 2) : (interval_s + interval - 1);

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("GS_PRE_CON_DOT_FW");
#endif
    // forward substitution
    for (j = 1; j < ySize - 1 + num_th - 1; ++j)
    {
      jj = j - th_id;
      if (jj >= 1 && jj < ySize - 1)
      {
        // #pragma omp simd
        for (i = interval_s; i <= interval_e; ++i)
        {
          (*x)(jj, i) = w_c * ((*rhs)(jj, i) + (w_y * (*x)(jj - 1, i) + w_x * (*x)(jj, i - 1)));
        }
      }
#pragma omp barrier
    }

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("GS_PRE_CON_DOT_FW");
#endif

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_START("GS_PRE_CON_DOT_BW");
#endif
    // backward substitution
    for (j = ySize - 2 + num_th - 1; j > 0; --j)
    {
      jj = j - th_id;
      if (jj < ySize - 1 && jj >= 1)
      {
        // #pragma omp simd
        for (i = interval_e; i >= interval_s; --i)
        {
          (*x)(jj, i) += w_c * (w_y * (*x)(jj + 1, i) + w_x * (*x)(jj, i + 1));
          dotprod += ((*x)(jj, i) * (*rhs)(jj, i));
        }
      }
#pragma omp barrier
    }

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_STOP("GS_PRE_CON_DOT_BW");
#endif
  }

  STOP_TIMER(GS_PRE_CON_DOT);
  return dotprod;
}

int PDE::solve(Grid *x, Grid *b, Solver type, int niter, double tol)
{
  SolverClass solver(this, x, b);
  if (type == CG)
  {
    return solver.CG(niter, tol);
  }
  else if (type == PCG)
  {
    return solver.PCG(niter, tol);
  }
  else
  {
    printf("Solver not existing\n");
    return -1;
  }
}
