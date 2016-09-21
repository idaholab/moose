/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatCapacityConductionTimeDerivative.h"

template<>
InputParameters validParams<HeatCapacityConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>("heat_capacity", "heat_capacity", "Property name of the heat capacity material property");
  params.addCoupledVar("args", "Vector of additional arguments of the heat capacity");
  return params;
}


HeatCapacityConductionTimeDerivative::HeatCapacityConductionTimeDerivative(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<TimeDerivative>>(parameters),
    _heat_capacity(getMaterialProperty<Real>("heat_capacity")),
    _d_heat_capacity_dT(getMaterialPropertyDerivative<Real>("heat_capacity", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _d_heat_capacity_dargs.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _d_heat_capacity_dargs[i] = &getMaterialPropertyDerivative<Real>("heat_capacity", _coupled_moose_vars[i]->name());
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
  return   _heat_capacity[_qp] * TimeDerivative::computeQpJacobian()
         + _d_heat_capacity_dT[_qp] * _phi[_j][_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatCapacityConductionTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // off-diagonal contribution with terms that depend on coupled variables
  return (*_d_heat_capacity_dargs[cvar])[_qp] * _phi[_j][_qp] * TimeDerivative::computeQpResidual();
}
