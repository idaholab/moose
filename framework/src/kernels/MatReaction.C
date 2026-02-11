//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatReaction.h"

registerMooseObject("MooseApp", MatReaction);
registerMooseObject("MooseApp", ADMatReaction);

template <bool is_ad>
InputParameters
MatReactionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Kernel to add -L*v, where L=reaction rate, v=variable");
  params.addCoupledVar("v",
                       "Set this to make v a coupled variable, otherwise it will use the "
                       "kernel's nonlinear variable for v");
  params.addRequiredParam<MaterialPropertyName>("reaction_rate",
                                                "The reaction rate used with the kernel");
  return params;
}

template <bool is_ad>
MatReactionTempl<is_ad>::MatReactionTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _rate(this->template getGenericMaterialProperty<Real, is_ad>("reaction_rate")),
    _v(this->isCoupled("v") ? this->template coupledGenericValue<is_ad>("v") : this->_u)
{
  if (this->isCoupled("v") && coupled("v") == _var.number())
    paramError("v", "In order to correctly couple the primal variable, leave 'v' blank");
}

template <bool is_ad>
GenericReal<is_ad>
MatReactionTempl<is_ad>::computeQpResidual()
{
  return -_rate[_qp] * _v[_qp] * _test[_i][_qp];
}

InputParameters
MatReaction::validParams()
{
  InputParameters params = MatReactionTempl<false>::validParams();
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

MatReaction::MatReaction(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<MatReactionTempl<false>>>(parameters),
    _is_coupled(isCoupled("v")),
    _v_name(_is_coupled ? coupledName("v") : _var.name()),
    _v_var(_is_coupled ? coupled("v") : _var.number()),
    _drate_du(getMaterialPropertyDerivative<Real>("reaction_rate", _var.name())),
    _drate_dv(getMaterialPropertyDerivative<Real>("reaction_rate", _v_name)),
    _drate_darg(_n_args)
{
  // Get reaction rate derivatives
  for (unsigned int i = 0; i < _n_args; ++i)
    _drate_darg[i] = &getMaterialPropertyDerivative<Real>("reaction_rate", i);
}

void
MatReaction::initialSetup()
{
  validateNonlinearCoupling<Real>("reaction_rate");
}

Real
MatReaction::computeQpJacobian()
{
  if (_is_coupled)
    return -_drate_du[_qp] * _v[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  return -(_rate[_qp] + _drate_du[_qp] * _v[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MatReaction::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first handle the case where jvar is a coupled variable v being added to residual
  // the first term in the sum just multiplies by L which is always needed
  // the second term accounts for cases where L depends on v
  if (jvar == _v_var && _is_coupled)
    return -(_rate[_qp] + _drate_dv[_qp] * _v[_qp]) * _test[_i][_qp] * _phi[_j][_qp];

  //  for all other vars get the coupled variable jvar is referring to
  const auto cvar = mapJvarToCvar(jvar);

  return -(*_drate_darg[cvar])[_qp] * _v[_qp] * _test[_i][_qp] * _phi[_j][_qp];
}

template class MatReactionTempl<false>;
template class MatReactionTempl<true>;
