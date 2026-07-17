//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowLumpedKernelBase.h"

template <bool is_ad>
PorousFlowLumpedKernelBaseTempl<is_ad>::PorousFlowLumpedKernelBaseTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters)
{
}

template <bool is_ad>
void
PorousFlowLumpedKernelBaseTempl<is_ad>::jacobianSetup()
{
  GenericKernel<is_ad>::jacobianSetup();
  _my_elem_lma = nullptr;
}

// AD-path Jacobian assembly for kernels that use mass-lumped (nodal) material properties.
//
// The default ADKernel path (ADKernel::computeADJacobian -> addJacobian) routes through
// Assembly::cacheJacobian, which takes the sparse column set from residuals[0] and reuses it for
// every row.  That assumes every test function's residual depends on the same set of DOFs, which
// holds for ordinary qp-based weak forms because the solution at any QP is sum_j u_j*phi_j(qp)
// and therefore involves every element DOF.
//
// With mass lumping each residual row _i depends only on the nodal material properties at node _i
// (i.e. on the DOFs at node _i alone), so the column set from residuals[0] is only valid for row 0;
// the off-node rows are assembled against the wrong columns and come out zero.
//
// addJacobianWithoutConstraints reads each row's own column indices from
// residuals[_i].derivatives().nude_indices(), giving the correct per-node block structure.
//
// This is consistent with how the framework handles analogous cases:
//   - ADNodalKernel::computeJacobian passes a size-1 residual array; Assembly::cacheJacobian's
//     residuals.size()==1 branch falls straight through to cacheJacobianWithoutConstraints.
//   - MassLumpedTimeDerivative::computeJacobian (non-AD) overrides assembly to fill only the
//     node-diagonal entries, for the same structural reason.
//
// constrain_element_matrix is therefore skipped.  It cannot be applied to a node-diagonal block
// structure: the constraint machinery assumes a fully-coupled local matrix where all rows share a
// column space.  Neither the non-AD lumped path nor the framework AD nodal path applies it either.
template <bool is_ad>
void
PorousFlowLumpedKernelBaseTempl<is_ad>::computeJacobian()
{
  if constexpr (!is_ad)
    GenericKernel<is_ad>::computeJacobian();
  else
  {
    if (_my_elem_lma != this->_current_elem)
    {
      this->computeResidualsForJacobian();
      this->addJacobianWithoutConstraints(
          this->_assembly, this->_residuals, this->dofIndices(), this->_var.scalingFactor());
      _my_elem_lma = this->_current_elem;
    }
  }
}

// A single AD residual evaluation carries derivatives wrt every coupled variable, so the
// off-diagonal blocks are assembled by the same addJacobianWithoutConstraints call as the diagonal.
// _my_elem_lma caches the element so computation happens once per element rather than once per
// coupled jvar, mirroring ADKernel's own _my_elem guard in ADKernel::computeOffDiagJacobian.
template <bool is_ad>
void
PorousFlowLumpedKernelBaseTempl<is_ad>::computeOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
    GenericKernel<is_ad>::computeOffDiagJacobian(jvar);
  else
  {
    libmesh_ignore(jvar);
    if (_my_elem_lma != this->_current_elem)
    {
      this->computeResidualsForJacobian();
      this->addJacobianWithoutConstraints(
          this->_assembly, this->_residuals, this->dofIndices(), this->_var.scalingFactor());
      _my_elem_lma = this->_current_elem;
    }
  }
}

template class PorousFlowLumpedKernelBaseTempl<false>;
template class PorousFlowLumpedKernelBaseTempl<true>;
