/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SpecificHeatConductionTimeDerivative.h"

template <>
InputParameters
validParams<SpecificHeatConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription(
      "Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation with the specific heat $c_p$ and the density $\\rho$ as arguments.");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "specific_heat", "specific_heat", "Property name of the specific heat material property");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Property name of the density material property");
  params.addCoupledVar("args", "Vector of additional arguments of the specific heat and density");
  return params;
}

SpecificHeatConductionTimeDerivative::SpecificHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>(parameters),
    _specific_heat(getMaterialProperty<Real>("specific_heat")),
    _d_specific_heat_dT(getMaterialPropertyDerivative<Real>("specific_heat", _var.name())),
    _density(getMaterialProperty<Real>("density")),
    _d_density_dT(getMaterialPropertyDerivative<Real>("density", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _d_specific_heat_dargs.resize(nvar);
  _d_density_dargs.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    const std::string iname = _coupled_moose_vars[i]->name();
    _d_specific_heat_dargs[i] = &getMaterialPropertyDerivative<Real>("specific_heat", iname);
    _d_density_dargs[i] = &getMaterialPropertyDerivative<Real>("density", iname);
  }
}

Real
SpecificHeatConductionTimeDerivative::computeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpResidual();
}

Real
SpecificHeatConductionTimeDerivative::computeQpJacobian()
{
  const Real dT = TimeDerivative::computeQpResidual();

  // on-diagonal Jacobian with all terms that may depend on the kernel variable
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpJacobian() +
         _d_specific_heat_dT[_qp] * _phi[_j][_qp] * _density[_qp] * dT +
         _specific_heat[_qp] * _d_density_dT[_qp] * _phi[_j][_qp] * dT;
}

Real
SpecificHeatConductionTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // off-diagonal contribution with terms that depend on coupled variables
  const Real dT = TimeDerivative::computeQpResidual();
  return (*_d_specific_heat_dargs[cvar])[_qp] * _phi[_j][_qp] * _density[_qp] * dT +
         _specific_heat[_qp] * (*_d_density_dargs[cvar])[_qp] * _phi[_j][_qp] * dT;
}
