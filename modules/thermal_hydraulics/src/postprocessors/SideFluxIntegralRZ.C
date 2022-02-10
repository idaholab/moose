//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFluxIntegralRZ.h"

registerMooseObject("ThermalHydraulicsApp", SideFluxIntegralRZ);

InputParameters
SideFluxIntegralRZ::validParams()
{
  InputParameters params = SideFluxIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Integrates a diffusive flux over a boundary of a 2D RZ domain.");

  return params;
}

SideFluxIntegralRZ::SideFluxIntegralRZ(const InputParameters & parameters)
  : SideFluxIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
SideFluxIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * SideFluxIntegral::computeQpIntegral();
}
