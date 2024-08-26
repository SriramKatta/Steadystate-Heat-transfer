#include "PDE.h"
#include <math.h>
#include <iostream>
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
  for (int i = 0; i < 4; ++i)
    boundary[i] = Dirichlet;

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

  int collimit = (1.25 * 1024 * 1024) / 48;
  int colend = 0;

#pragma omp parallel
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
        for (int i = colstart; i < colend; ++i)
        {
          (*lhs)(j, i) = w_c * (*x)(j, i) - w_y * ((*x)(j + 1, i) + (*x)(j - 1, i)) - w_x * ((*x)(j, i + 1) + (*x)(j, i - 1));
        }
      }
    }
  }

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_STOP("APPLY_STENCIL");
#endif

  STOP_TIMER(APPLY_STENCIL);
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

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_START("GS_PRE_CON");
#endif

  // forward substitution
  for (int j = 1; j < ySize - 1; ++j)
  {
    for (int i = 1; i < xSize - 1; ++i)
    {
      (*x)(j, i) = w_c * ((*rhs)(j, i) + (w_y * (*x)(j - 1, i) + w_x * (*x)(j, i - 1)));
    }
  }
  // backward substitution
  for (int j = ySize - 2; j > 0; --j)
  {
    for (int i = xSize - 2; i > 0; --i)
    {
      (*x)(j, i) = (*x)(j, i) + w_c * (w_y * (*x)(j + 1, i) + w_x * (*x)(j, i + 1));
    }
  }

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_STOP("GS_PRE_CON");
#endif

  STOP_TIMER(GS_PRE_CON);
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
