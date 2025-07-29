//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicInputTimes.h"

registerMooseObject("MooseApp", DynamicInputTimes);

InputParameters
DynamicInputTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Times set dynamically during the simulation.");
  params.addRequiredParam<Real>("nexttime", "Time to store in the times vector");
  params.declareControllable("nexttime");
  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;

  return params;
}

DynamicInputTimes::DynamicInputTimes(const InputParameters & parameters)
  : Times(parameters), _nexttime(getParam<Real>("nexttime"))
{
  // Initialize times array
  _times.push_back(0);
}

void
DynamicInputTimes::initialize()
{

  _times.push_back(_nexttime);
}
