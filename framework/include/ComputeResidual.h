#ifndef COMPUTERESIDUAL_H
#define COMPUTERESIDUAL_H

namespace Moose
{
  /**
   * For use by a NonlinearSolver for computing the current residual.
   */
  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual);
}

#endif //COMPUTERESIDUAL_H
