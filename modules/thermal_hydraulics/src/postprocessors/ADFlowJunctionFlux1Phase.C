//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFlowJunctionFlux1Phase.h"
#include "ADFlowJunctionUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ADFlowJunctionFlux1Phase);

InputParameters
ADFlowJunctionFlux1Phase::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredParam<unsigned int>("connection_index", "Index of the connected flow channel");
  params.addRequiredParam<std::string>("junction", "Junction component name");
  MooseEnum equation("mass=0 momentum=1 energy=2");
  params.addRequiredParam<MooseEnum>(
      "equation", equation, "Equation for which to query flux vector");

  params.addClassDescription(
      "Retrieves an entry of a flux vector for a connection attached to a 1-phase junction");

  return params;
}

ADFlowJunctionFlux1Phase::ADFlowJunctionFlux1Phase(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _connection_index(getParam<unsigned int>("connection_index")),
    _junction_name(getParam<std::string>("junction")),
    _junction_uo_name(_junction_name + ":junction_uo"),
    _junction_uo(getUserObjectByName<ADFlowJunctionUserObject>(_junction_uo_name)),
    _equation_index(getParam<MooseEnum>("equation"))
{
}

Real
ADFlowJunctionFlux1Phase::computeQpIntegral()
{
  const auto & flux = _junction_uo.getFlux(_connection_index);
  return MetaPhysicL::raw_value(flux[_equation_index]);
}
