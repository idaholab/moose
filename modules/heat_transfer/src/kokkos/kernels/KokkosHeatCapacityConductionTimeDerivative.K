//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatCapacityConductionTimeDerivative.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHeatCapacityConductionTimeDerivative);

InputParameters
KokkosHeatCapacityConductionTimeDerivative::validParams()
{
  InputParameters params = KokkosTimeDerivative::validParams();
  params.addClassDescription("Time derivative term $C_p \\frac{\\partial T}{\\partial t}$ of "
                             "the heat equation with the heat capacity $C_p$ as an argument.");
  params.addCoupledVar("coupled_variables",
                       "Vector of nonlinear variable arguments this object depends on");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "heat_capacity", "heat_capacity", "Property name of the heat capacity material property");
  return params;
}

KokkosHeatCapacityConductionTimeDerivative::KokkosHeatCapacityConductionTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<KokkosTimeDerivative>(parameters),
    _heat_capacity(getKokkosMaterialProperty<Real>("heat_capacity")),
    _d_heat_capacity_dT(getKokkosMaterialPropertyDerivative<Real>("heat_capacity", _var.name()))
{
  const auto & vars = getCoupledMooseVars();

  _d_heat_capacity_dargs.create(vars.size());

  for (const auto i : index_range(vars))
  {
    _jvar_to_index[vars[i]->number()] = i;
    _d_heat_capacity_dargs[i] =
        getKokkosMaterialPropertyDerivative<Real>("heat_capacity", vars[i]->name());
  }

  _jvar_to_index.copyToDevice();
  _d_heat_capacity_dargs.copyToDevice();
}
