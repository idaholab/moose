//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateRadiationRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateRadiationRZ);

InputParameters
HeatRateRadiationRZ::validParams()
{
  InputParameters params = HeatRateRadiation::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary radiative heat flux");

  return params;
}

HeatRateRadiationRZ::HeatRateRadiationRZ(const InputParameters & parameters)
  : HeatRateRadiation(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatRateRadiationRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatRateRadiation::computeQpIntegral();
}
