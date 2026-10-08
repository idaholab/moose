//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTrussHeatConductionTimeDerivative.h"

registerKokkosResidualObject("HeatTransferApp", KokkosTrussHeatConductionTimeDerivative);

InputParameters
KokkosTrussHeatConductionTimeDerivative::validParams()
{
  InputParameters params = KokkosHeatConductionTimeDerivative::validParams();
  params.addClassDescription("Computes time derivative term in heat equation for truss elements, "
                             "taking cross-sectional area into account");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  return params;
}

KokkosTrussHeatConductionTimeDerivative::KokkosTrussHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : KokkosHeatConductionTimeDerivative(parameters), _area(kokkosCoupledValue("area"))
{
}
