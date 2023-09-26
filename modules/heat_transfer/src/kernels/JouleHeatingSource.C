//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JouleHeatingSource.h"

registerMooseObject("HeatConductionApp", JouleHeatingSource);

InputParameters
JouleHeatingSource::validParams()
{
  InputParameters params =
      DerivativeMaterialInterface<JvarMapKernelInterface<HeatSource>>::validParams();
  params.addCoupledVar("elec", "Electric potential for joule heating.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  params.addClassDescription("Calculates the heat source term corresponding to electrostatic Joule "
                             "heating.");
  return params;
}

JouleHeatingSource::JouleHeatingSource(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<HeatSource>>(parameters),
    _grad_elec(coupledGradient("elec")),
    _elec_var(coupled("elec")),
    _elec_cond(getMaterialProperty<Real>("electrical_conductivity")),
    _delec_cond_dT(getMaterialPropertyDerivative<Real>("electrical_conductivity", _var.name())),
    _delec_cond_darg(_coupled_moose_vars.size())
{
  for (unsigned int i = 0; i < _delec_cond_darg.size(); ++i)
    _delec_cond_darg[i] = &getMaterialPropertyDerivative<Real>("electrical_conductivity",
                                                               _coupled_moose_vars[i]->name());
}

void
JouleHeatingSource::initialSetup()
{
  validateNonlinearCoupling<Real>("electrical_conductivity");
}

Real
JouleHeatingSource::computeQpResidual()
{
  return -_elec_cond[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _test[_i][_qp];
}

Real
JouleHeatingSource::computeQpJacobian()
{
  return -_delec_cond_dT[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
JouleHeatingSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _elec_var)
    return -2 * _elec_cond[_qp] * _grad_elec[_qp] * _grad_phi[_j][_qp] * _test[_i][_qp] -
           (*_delec_cond_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
               _test[_i][_qp];

  return -(*_delec_cond_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
         _test[_i][_qp];
}
