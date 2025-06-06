#include "Solver.h"
#include "Grid.h"
#ifdef LIKWID_PERFMON
#include <likwid.h>
#endif

#define IS_VALID(a) \
  !(std::isnan(a) || std::isinf(a))

SolverClass::SolverClass(PDE *pde_, Grid *x_, Grid *b_) : pde(pde_), x(x_), b(b_)
{
}

int SolverClass::CG(int niter, double tol)
{
  Grid *p = new Grid(pde->numGrids_x(), pde->numGrids_y());
  Grid *v = new Grid(pde->numGrids_x(), pde->numGrids_y());

  int iter = 0;
  double lambda = 0;
  double alpha_0 = 0, alpha_1 = 0;
  // Calculate residual
  // p=A*x
  pde->applyStencil(p, x);
  // p=b-A*x
  axpby(p, 1, b, -1, p);
  // calculate alpha_0
  alpha_0 = dotProduct(p, p);

  Grid *r = new Grid(*p);

  START_TIMER(CG);
#ifdef LIKWID_PERFMON
  LIKWID_MARKER_START("CG_Solver");
#endif
  while ((iter < niter) && (alpha_0 > tol * tol))
  {
    const auto vpdot = pde->applyStencil_dot(v, p);
    lambda = alpha_0 / vpdot;
    // Update r
    alpha_1 = axpby_dot(r, 1.0, r, -lambda, v);
    // dotProduct(r, r);
    // Update x
    //  Update p
    // axpby(x, 1.0, x, lambda, p);
    // axpby(p, 1.0, r, alpha_1 / alpha_0, p);
    axpby_fused_xp(x, p, r, lambda, alpha_1 / alpha_0);
    alpha_0 = alpha_1;
#ifdef DEBUG
    printf("iter = %d, res = %.15e\n", iter, alpha_0);
#endif
    ++iter;
  }

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_STOP("CG_Solver");
#endif
  STOP_TIMER(CG);

  if (!IS_VALID(alpha_0))
  {
    printf("\x1B[31mWARNING: NaN/INF detected after iteration %d\x1B[0m\n", iter);
  }

  delete p;
  delete v;
  delete r;

  return (iter);
}

int SolverClass::PCG(int niter, double tol)
{
  Grid *r = new Grid(pde->numGrids_x(), pde->numGrids_y());
  Grid *z = new Grid(pde->numGrids_x(), pde->numGrids_y());
  Grid *v = new Grid(pde->numGrids_x(), pde->numGrids_y());

  int iter = 0;
  double lambda = 0;
  double alpha_0 = 0, alpha_1 = 0;
  double res_norm_sq = 0;
  // Calculate residual
  pde->applyStencil(r, x);
  axpby(r, 1, b, -1, r);
  res_norm_sq = dotProduct(r, r);
  pde->GSPreCon(r, z);

  alpha_0 = dotProduct(r, z);
  Grid *p = new Grid(*z);

  START_TIMER(PCG);

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_START("PCG_Solver");
#endif
  while ((iter < niter) && (res_norm_sq > tol * tol))
  {
    const auto vpdot = pde->applyStencil_dot(v, p);
    lambda = alpha_0 / vpdot;
    // Update r
    res_norm_sq = axpby_dot(r, 1.0, r, -lambda, v);
    // dotProduct(r, r);
    // axpby(r, 1.0, r, -lambda, v);
    // res_norm_sq = dotProduct(r, r);
    //  Update z
    alpha_1 = pde->GSPreCon_dot(r, z);
    // dotProduct(r, z);
    //  Update p
    // Update x
    // axpby(x, 1.0, x, lambda, p);
    // axpby(p, 1.0, z, alpha_1 / alpha_0, p);
    axpby_fused_xp(x, p, z, lambda, alpha_1 / alpha_0);
    alpha_0 = alpha_1;

#ifdef DEBUG
    printf("iter = %d, res = %.15e\n", iter, res_norm_sq);
#endif
    ++iter;
  }

#ifdef LIKWID_PERFMON
  LIKWID_MARKER_STOP("PCG_Solver");
#endif
  STOP_TIMER(PCG);

  if (!IS_VALID(res_norm_sq))
  {
    printf("\x1B[31mWARNING: NaN/INF detected after iteration %d\x1B[0m\n", iter);
  }

  delete r;
  delete z;
  delete v;
  delete p;

  return (iter);
}
