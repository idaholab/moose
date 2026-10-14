//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleCoupledACInterface.h"

registerMooseObject("PhaseFieldApp", SimpleCoupledACInterface);
registerMooseObject("PhaseFieldApp", ADSimpleCoupledACInterface);

template <bool is_ad>
InputParameters
SimpleCoupledACInterfaceTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Gradient energy for Allen-Cahn Kernel with constant Mobility and "
                             "Interfacial parameter for a coupled order parameter variable.");
  params.addRequiredCoupledVar("v", "Coupled variable that the Laplacian is taken of");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  return params;
}

template <bool is_ad>
SimpleCoupledACInterfaceTempl<is_ad>::SimpleCoupledACInterfaceTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _L(this->template getGenericMaterialProperty<Real, is_ad>("mob_name")),
    _kappa(this->template getGenericMaterialProperty<Real, is_ad>("kappa_name")),
    _grad_v(this->template coupledGenericGradient<is_ad>("v"))
{
}

template <bool is_ad>
GenericReal<is_ad>
SimpleCoupledACInterfaceTempl<is_ad>::computeQpResidual()
{
  return _grad_v[_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];
}

SimpleCoupledACInterface::SimpleCoupledACInterface(const InputParameters & parameters)
  : SimpleCoupledACInterfaceTempl<false>(parameters), _v_var(coupled("v", 0))
{
}

Real
SimpleCoupledACInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _grad_phi[_j][_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];

  return 0.0;
}

template class SimpleCoupledACInterfaceTempl<false>;
template class SimpleCoupledACInterfaceTempl<true>;
