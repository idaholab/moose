//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSpecificHeatConductionTimeDerivative.h"

registerKokkosResidualObject("HeatTransferApp", KokkosSpecificHeatConductionTimeDerivative);

InputParameters
KokkosSpecificHeatConductionTimeDerivative::validParams()
{
  InputParameters params = KokkosTimeDerivative::validParams();
  params.addClassDescription(
      "Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation with the specific heat $c_p$ and the density $\\rho$ as arguments.");
  params.addCoupledVar("coupled_variables",
                       "Vector of nonlinear variable arguments this object depends on");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "specific_heat", "specific_heat", "Property name of the specific heat material property");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Property name of the density material property");
  return params;
}

KokkosSpecificHeatConductionTimeDerivative::KokkosSpecificHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<KokkosTimeDerivative>(parameters),
    _specific_heat(getKokkosMaterialProperty<Real>("specific_heat")),
    _d_specific_heat_dT(getKokkosMaterialPropertyDerivative<Real>("specific_heat", _var.name())),
    _density(getKokkosMaterialProperty<Real>("density")),
    _d_density_dT(getKokkosMaterialPropertyDerivative<Real>("density", _var.name()))
{
  const auto & vars = getCoupledMooseVars();

  _d_specific_heat_dargs.create(vars.size());
  _d_density_dargs.create(vars.size());

  for (const auto i : index_range(vars))
  {
    const auto & name = vars[i]->name();
    _jvar_to_index[vars[i]->number()] = i;
    _d_specific_heat_dargs[i] = getKokkosMaterialPropertyDerivative<Real>("specific_heat", name);
    _d_density_dargs[i] = getKokkosMaterialPropertyDerivative<Real>("density", name);
  }

  _jvar_to_index.copyToDevice();
  _d_specific_heat_dargs.copyToDevice();
  _d_density_dargs.copyToDevice();
}
