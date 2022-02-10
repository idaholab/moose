//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateConvectionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConvectionRZ);

InputParameters
HeatRateConvectionRZ::validParams()
{
  InputParameters params = HeatRateConvection::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary convective heat flux");

  return params;
}

HeatRateConvectionRZ::HeatRateConvectionRZ(const InputParameters & parameters)
  : HeatRateConvection(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatRateConvectionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatRateConvection::computeQpIntegral();
}
