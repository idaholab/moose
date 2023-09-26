//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatCapacityConductionTimeDerivative.h"

registerMooseObject("HeatConductionApp", HeatCapacityConductionTimeDerivative);

InputParameters
HeatCapacityConductionTimeDerivative::validParams()
{
  InputParameters params = JvarMapKernelInterface<TimeDerivative>::validParams();
  params.addClassDescription("Time derivative term $C_p \\frac{\\partial T}{\\partial t}$ of "
                             "the heat equation with the heat capacity $C_p$ as an argument.");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "heat_capacity", "heat_capacity", "Property name of the heat capacity material property");
  return params;
}

HeatCapacityConductionTimeDerivative::HeatCapacityConductionTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>(parameters),
    _heat_capacity(getMaterialProperty<Real>("heat_capacity")),
    _d_heat_capacity_dT(getMaterialPropertyDerivative<Real>("heat_capacity", _var.name()))
{
  // get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _d_heat_capacity_dargs.resize(nvar);

  // iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _d_heat_capacity_dargs[i] =
        &getMaterialPropertyDerivative<Real>("heat_capacity", _coupled_moose_vars[i]->name());
}

Real
HeatCapacityConductionTimeDerivative::computeQpResidual()
{
  return _heat_capacity[_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatCapacityConductionTimeDerivative::computeQpJacobian()
{
  // on-diagonal Jacobian with all terms that may depend on the kernel variable
  return _heat_capacity[_qp] * TimeDerivative::computeQpJacobian() +
         _d_heat_capacity_dT[_qp] * _phi[_j][_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatCapacityConductionTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // off-diagonal contribution with terms that depend on coupled variables
  return (*_d_heat_capacity_dargs[cvar])[_qp] * _phi[_j][_qp] * TimeDerivative::computeQpResidual();
}
