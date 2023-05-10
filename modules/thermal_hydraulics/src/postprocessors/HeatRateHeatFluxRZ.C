//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateHeatFluxRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateHeatFluxRZ);

InputParameters
HeatRateHeatFluxRZ::validParams()
{
  InputParameters params = HeatRateHeatFlux::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a heat flux function over a cylindrical boundary in a XYZ coordinate system.");

  return params;
}

HeatRateHeatFluxRZ::HeatRateHeatFluxRZ(const InputParameters & parameters)
  : HeatRateHeatFlux(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatRateHeatFluxRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatRateHeatFlux::computeQpIntegral();
}
