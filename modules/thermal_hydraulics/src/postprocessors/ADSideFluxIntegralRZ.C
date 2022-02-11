//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSideFluxIntegralRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADSideFluxIntegralRZ);

InputParameters
ADSideFluxIntegralRZ::validParams()
{
  InputParameters params = ADSideFluxIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Integrates a diffusive flux over a boundary of a 2D RZ domain.");

  return params;
}

ADSideFluxIntegralRZ::ADSideFluxIntegralRZ(const InputParameters & parameters)
  : ADSideFluxIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
ADSideFluxIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADSideFluxIntegral::computeQpIntegral();
}
