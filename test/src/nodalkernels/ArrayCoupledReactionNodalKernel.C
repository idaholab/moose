//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayCoupledReactionNodalKernel.h"

registerMooseObject("MooseTestApp", ArrayCoupledReactionNodalKernel);

InputParameters
ArrayCoupledReactionNodalKernel::validParams()
{
  InputParameters params = ArrayNodalKernel::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term at nodes for an array variable");
  params.addRequiredParam<RealEigenVector>(
      "coeff",
      "Coefficients for pairwise reactions between adjacent array entries (should have one less "
      "than count entries)");
  return params;
}

ArrayCoupledReactionNodalKernel::ArrayCoupledReactionNodalKernel(const InputParameters & parameters)
  : ArrayNodalKernel(parameters), _coeff(getParam<RealEigenVector>("coeff"))
{
  if (_coeff.size() != _count - 1)
    paramError(
        "coeff",
        "The size of the coefficient vector must match the size of the array variable minus one");
}

void
ArrayCoupledReactionNodalKernel::computeQpResidual(RealEigenVector & residual)
{
  for (const auto i : make_range(_count))
    residual(i) = (i + 1 < _count ? -_u[_qp](i) * _coeff(i) + _u[_qp](i + 1) * _coeff(i) : 0.0) +
                  (i > 0 ? (_u[_qp](i - 1) * _coeff(i - 1) - _u[_qp](i) * _coeff(i - 1)) : 0.0);
}

void
ArrayCoupledReactionNodalKernel::computeQpJacobian()
{
  for (const auto i : make_range(_count))
  {
    Real diag = 0.0;
    if (i > 0)
      diag -= _coeff(i - 1);
    if (i + 1 < _count)
      diag -= _coeff(i);
    setJacobian(i, i, diag);

    if (i + 1 < _count)
      setJacobian(i, i + 1, _coeff(i));
    if (i > 0)
      setJacobian(i, i - 1, _coeff(i - 1));
  }
}

void
ArrayCoupledReactionNodalKernel::getJacobianComponentCoupling(
    std::vector<std::pair<unsigned int, unsigned int>> & coupling) const
{
  if (_count < 2)
    return;

  /*
  * Every (i,j) pair that is used in a computeQpJacobian setJacobian(i, j, value)
  * call must be added to the coupling list to ensure a correct and complete
  * sparsity pattern.
  * TODO: Sparsity pattern definition for inter-variable coupling!
  */
  coupling.reserve(2 * (_count - 1));
  for (unsigned int i = 0; i + 1 < _count; ++i)
    coupling.emplace_back(i, i + 1);
  for (unsigned int i = 1; i < _count; ++i)
    coupling.emplace_back(i, i - 1);
}
