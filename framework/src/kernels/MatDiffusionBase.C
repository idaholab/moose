//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDiffusionBase.h"

template <typename T, bool is_ad>
InputParameters
MatDiffusionBaseTempl<T, is_ad>::validParams()
{
  InputParameters params = GenericKernelGrad<is_ad>::validParams();
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}

template <typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = MatDiffusionBaseTempl<T, false>::validParams();
  params.addCoupledVar("args",
                       "Optional vector of arguments for the diffusivity. If provided and "
                       "diffusivity is a derivative parsed material, Jacobian contributions from "
                       "the diffusivity will be automatically computed");
  return params;
}

template <typename T, bool is_ad>
MatDiffusionBaseTempl<T, is_ad>::MatDiffusionBaseTempl(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernelGrad<is_ad>>>(parameters),
    _diffusivity(this->template getGenericMaterialProperty<T, is_ad>("diffusivity")),
    _grad_v(isCoupled("v") ? this->template coupledGenericGradient<is_ad>("v") : _grad_u)
{
}

template <typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters)
  : MatDiffusionBaseTempl<T, false>(parameters),
    _ddiffusivity_dc(this->template getMaterialPropertyDerivative<T>("diffusivity", _var.name())),
    _ddiffusivity_darg(_coupled_moose_vars.size()),
    _is_coupled(this->isCoupled("v")),
    _v_var(_is_coupled ? this->coupled("v") : _var.number())
{
  // fetch derivatives
  for (unsigned int i = 0; i < _ddiffusivity_darg.size(); ++i)
    _ddiffusivity_darg[i] = &this->template getMaterialPropertyDerivative<T>(
        "diffusivity", _coupled_moose_vars[i]->name());
}

template <typename T, bool is_ad>
GenericRealVectorValue<is_ad>
MatDiffusionBaseTempl<T, is_ad>::precomputeQpResidual()
{
  return _diffusivity[_qp] * _grad_v[_qp];
}

template <typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  this->template validateNonlinearCoupling<T>("diffusivity");
}

template <typename T>
RealGradient
MatDiffusionBase<T>::computeQpCJacobian()
{
  return _diffusivity[_qp] * _grad_phi[_j][_qp];
}

template <typename T>
RealGradient
MatDiffusionBase<T>::precomputeQpJacobian()
{
  auto sum = _ddiffusivity_dc[_qp] * _phi[_j][_qp] * _grad_v[_qp];
  if (!_is_coupled)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const auto cvar = this->mapJvarToCvar(jvar);

  auto sum = (*_ddiffusivity_darg[cvar])[_qp] * _phi[_j][_qp] * _grad_v[_qp];
  if (_v_var == jvar)
    sum += computeQpCJacobian();

  return sum * _grad_test[_i][_qp];
}

template class MatDiffusionBase<Real>;
template class MatDiffusionBase<RealTensorValue>;
template class MatDiffusionBaseTempl<Real, false>;
template class MatDiffusionBaseTempl<RealTensorValue, false>;
template class MatDiffusionBaseTempl<Real, true>;
template class MatDiffusionBaseTempl<RealTensorValue, true>;
