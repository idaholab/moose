//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementMaxLevelPostProcessor.h"

registerMooseObject("MooseApp", ElementMaxLevelPostProcessor);

InputParameters
ElementMaxLevelPostProcessor::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription(
      "Computes the maximum element adaptivity level (for either h or p refinement).");
  params.addRequiredParam<MooseEnum>(
      "level", MooseEnum("h p"), "The type of adaptivity level to compute.");
  return params;
}

ElementMaxLevelPostProcessor::ElementMaxLevelPostProcessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters), _level_type(getParam<MooseEnum>("level").getEnum<LevelType>())
{
}

void
ElementMaxLevelPostProcessor::initialize()
{
  _max_level = 0;
}

void
ElementMaxLevelPostProcessor::execute()
{
#ifdef LIBMESH_ENABLE_AMR
  switch (_level_type)
  {
    case LevelType::H:
      _max_level = std::max(_max_level, _current_elem->level());
      break;
    case LevelType::P:
      _max_level = std::max(_max_level, _current_elem->p_level());
      break;
    default:
      break;
  }
#endif
}

Real
ElementMaxLevelPostProcessor::getValue() const
{
  return _max_level;
}

void
ElementMaxLevelPostProcessor::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const ElementMaxLevelPostProcessor &>(y);
  _max_level = std::max(_max_level, pps._max_level);
}

void
ElementMaxLevelPostProcessor::finalize()
{
  gatherMax(_max_level);
}
