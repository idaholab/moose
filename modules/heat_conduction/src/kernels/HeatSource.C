//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSource.h"

registerMooseObject("HeatConductionApp", HeatSource);

InputParameters
HeatSource::validParams()
{
  InputParameters params = BodyForce::validParams();

  // Override defaults and documentation, weak form is identical to BodyForce in MOOSE
  params.addParam<Real>("value", 1.0, "Value of heat source. Multiplied by function if present.");
  params.addParam<FunctionName>("function", "1", "Function describing the volumetric heat source");
  return params;
}

HeatSource::HeatSource(const InputParameters & parameters) : BodyForce(parameters) {}
