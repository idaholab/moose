//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BooleanValueTestComponent.h"

registerMooseObject("ThermalHydraulicsTestApp", BooleanValueTestComponent);

InputParameters
BooleanValueTestComponent::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<bool>("value", "A controllable boolean value");
  params.declareControllable("value");
  params.addClassDescription("Test component with a controllable boolean parameter");
  return params;
}

BooleanValueTestComponent::BooleanValueTestComponent(const InputParameters & params)
  : Component(params), _value(getParam<bool>("value"))
{
}

void
BooleanValueTestComponent::addMooseObjects()
{
  const std::string class_name = "BooleanValueTestUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<bool>("value") = _value;
  const std::string nm = genName(name(), "value_uo");
  getTHMProblem().addUserObject(class_name, nm, params);
  connectObject(params, nm, "value");
}
