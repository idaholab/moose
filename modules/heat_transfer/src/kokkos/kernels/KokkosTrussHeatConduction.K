//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTrussHeatConduction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosTrussHeatConduction);

InputParameters
KokkosTrussHeatConduction::validParams()
{
  InputParameters params = KokkosHeatConduction::validParams();
  params.addClassDescription("Computes conduction term in heat equation for truss elements, taking "
                             "cross-sectional area into account");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  return params;
}

KokkosTrussHeatConduction::KokkosTrussHeatConduction(const InputParameters & parameters)
  : KokkosHeatConduction(parameters), _area(kokkosCoupledValue("area"))
{
}
