#ifndef COMPUTEJACOBIAN_H
#define COMPUTEJACOBIAN_H

namespace Moose
{
  /**
   * Computes a block diagonal jacobian for the full system.
   */
  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian);

  /**
   * Computes one block of the jacobian.
   *
   * @param ivar The block row to compute.
   * @param jvar The block column to compute.
   */
  void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);
}

#endif //COMPUTEJACOBIAN_H


