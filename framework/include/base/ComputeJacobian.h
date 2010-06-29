#ifndef COMPUTEJACOBIAN_H
#define COMPUTEJACOBIAN_H

#include "numeric_vector.h"
#include "nonlinear_implicit_system.h"

namespace Moose
{
  /**
   * Computes a block diagonal jacobian for the full system.
   */
  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, NonlinearImplicitSystem& sys);

  /**
   * Computes one block of the jacobian.
   *
   * @param ivar The block row to compute.
   * @param jvar The block column to compute.
   */
  void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, NonlinearImplicitSystem& sys, unsigned int ivar, unsigned int jvar);
}

#endif //COMPUTEJACOBIAN_H


