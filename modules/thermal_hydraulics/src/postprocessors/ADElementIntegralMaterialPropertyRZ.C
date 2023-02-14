//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADElementIntegralMaterialPropertyRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADElementIntegralMaterialPropertyRZ);

InputParameters
ADElementIntegralMaterialPropertyRZ::validParams()
{
  InputParameters params = ADElementIntegralMaterialProperty::validParams();
  params += RZSymmetry::validParams();
  params.addClassDescription(
      "Computes the volume integral of a material property for an RZ geometry.");
  return params;
}

ADElementIntegralMaterialPropertyRZ::ADElementIntegralMaterialPropertyRZ(
    const InputParameters & parameters)
  : ADElementIntegralMaterialProperty(parameters), RZSymmetry(this, parameters)
{
}

Real
ADElementIntegralMaterialPropertyRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADElementIntegralMaterialProperty::computeQpIntegral();
}
