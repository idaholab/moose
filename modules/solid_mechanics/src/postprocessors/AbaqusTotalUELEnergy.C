//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusTotalUELEnergy.h"
#include "AbaqusUserElement.h"

registerMooseObject("SolidMechanicsApp", AbaqusTotalUELEnergy);

InputParameters
AbaqusTotalUELEnergy::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription(
      "Computes and reports the sum of a UEL energy component over the mesh.");
  params.addParam<std::vector<UserObjectName>>("uel_user_object",
                                               "AbaqusUserElement user objects.");
  params.addRequiredRangeCheckedParam<std::size_t>(
      "component", "component < 8", "Zero-based index into the 8 element energy array.");
  return params;
}

AbaqusTotalUELEnergy::AbaqusTotalUELEnergy(const InputParameters & parameters)
  : ElementPostprocessor(parameters), _component(getParam<std::size_t>("component"))
{
  const auto & uel_names = getParam<std::vector<UserObjectName>>("uel_user_object");
  for (const auto & name : uel_names)
    _uel_uo.push_back(&getUserObjectByName<AbaqusUserElement>(name));
}

void
AbaqusTotalUELEnergy::initialize()
{
  _sum = 0.0;
}

void
AbaqusTotalUELEnergy::execute()
{
  for (const auto uo : _uel_uo)
  {
    const auto * energy = uo->getUELEnergy(_current_elem->id());
    if (energy)
    {
      _sum += (*energy)[_component];
      return;
    }
  }

  mooseError("No energy data found for element ",
             _current_elem->id(),
             ". Did you set use_energy=true in the AbaqusUserElement object?");
}

void
AbaqusTotalUELEnergy::threadJoin(const UserObject & y)
{
  const auto & ate = static_cast<const AbaqusTotalUELEnergy &>(y);
  _sum += ate._sum;
}

void
AbaqusTotalUELEnergy::finalize()
{
  gatherSum(_sum);
}

Real
AbaqusTotalUELEnergy::getValue() const
{
  return _sum;
}
