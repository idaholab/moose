#ifndef COMPUTERESIDUAL_H
#define COMPUTERESIDUAL_H

#include "numeric_vector.h"
#include "nonlinear_implicit_system.h"

namespace Moose
{
  /**
   * For use by a NonlinearSolver for computing the current residual.
   */
  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys);
}

#endif //COMPUTERESIDUAL_H
