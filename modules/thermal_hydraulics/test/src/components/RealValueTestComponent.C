//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RealValueTestComponent.h"

registerMooseObject("ThermalHydraulicsTestApp", RealValueTestComponent);

InputParameters
RealValueTestComponent::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<Real>("value", "A controllable real value");
  params.declareControllable("value");
  params.addClassDescription("Test component with a controllable real parameter");
  return params;
}

RealValueTestComponent::RealValueTestComponent(const InputParameters & params)
  : Component(params), _value(getParam<Real>("value"))
{
}

void
RealValueTestComponent::addMooseObjects()
{
  const std::string class_name = "RealValueTestUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("value") = _value;
  const std::string nm = genName(name(), "value_uo");
  getTHMProblem().addUserObject(class_name, nm, params);
  connectObject(params, nm, "value");
}
