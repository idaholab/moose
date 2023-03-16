//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatGradSquareCoupled.h"

registerMooseObject("PhaseFieldApp", MatGradSquareCoupled);

InputParameters
MatGradSquareCoupled::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gradient square of a coupled variable.");
  params.addCoupledVar("elec_potential", "Electric potential");
  params.addCoupledVar("args", "Vector of variable arguments to prefactor");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  params.addParam<MaterialPropertyName>(
      "prefactor",
      "prefactor",
      "Material property providing a prefactor of electric potential contribution");
  return params;
}

MatGradSquareCoupled::MatGradSquareCoupled(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _grad_elec_potential(coupledGradient("elec_potential")),
    _elec_potential_var(coupled("elec_potential")),
    _prefactor(getMaterialProperty<Real>("prefactor")),
    _dprefactor_dphi(getMaterialPropertyDerivative<Real>("prefactor", _var.name())),
    _dprefactor_darg(_n_args)
{
  for (unsigned int i = 0; i < _n_args; ++i)
    _dprefactor_darg[i] = &getMaterialPropertyDerivative<Real>("prefactor", i);
}

void
MatGradSquareCoupled::initialSetup()
{
  validateNonlinearCoupling<Real>("prefactor");
}

Real
MatGradSquareCoupled::computeQpResidual()
{
  return -_prefactor[_qp] * _grad_elec_potential[_qp] * _grad_elec_potential[_qp] * _test[_i][_qp];
}

Real
MatGradSquareCoupled::computeQpJacobian()
{
  return -_dprefactor_dphi[_qp] * _grad_elec_potential[_qp] * _grad_elec_potential[_qp] *
         _phi[_j][_qp] * _test[_i][_qp];
}

Real
MatGradSquareCoupled::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _elec_potential_var)
    return -2 * _prefactor[_qp] * _grad_elec_potential[_qp] * _grad_phi[_j][_qp] * _test[_i][_qp] -
           (*_dprefactor_darg[cvar])[_qp] * _grad_elec_potential[_qp] * _grad_elec_potential[_qp] *
               _phi[_j][_qp] * _test[_i][_qp];

  return -(*_dprefactor_darg[cvar])[_qp] * _grad_elec_potential[_qp] * _grad_elec_potential[_qp] *
         _phi[_j][_qp] * _test[_i][_qp];
}
