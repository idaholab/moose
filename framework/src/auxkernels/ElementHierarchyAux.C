//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAdaptivityLevelAux.h"

registerMooseObject("MooseApp", ElementAdaptivityLevelAux);

InputParameters
ElementAdaptivityLevelAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("stores the element hierarchy in a aux variable");
  params.addRequiredParam<MooseEnum>(
      "level", MooseEnum("h p"), "The type of adaptivity level to compute.");

  return params;
}

ElementAdaptivityLevelAux::ElementAdaptivityLevelAux(const InputParameters & parameters)
  : AuxKernel(parameters), _level_type(getParam<MooseEnum>("level").getEnum<LevelType>())
{
}

Real
ElementAdaptivityLevelAux::computeValue()
{
  // Only check if the user asked to compute p level
  // if it's not p level then presume that h adaptivity level was asked
  switch (_level_type)
  {
    case LevelType::P:
      return static_cast<Real>(_current_elem->p_level());
    default:
      return static_cast<Real>(_current_elem->level());
  }
}
