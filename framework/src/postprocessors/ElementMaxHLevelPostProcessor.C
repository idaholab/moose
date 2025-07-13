//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementMaxHLevelPostProcessor.h"

registerMooseObject("MooseApp", ElementMaxHLevelPostProcessor);

InputParameters
ElementMaxHLevelPostProcessor::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription("Computes the maximum element h-level over the whole domain.");
  return params;
}

ElementMaxHLevelPostProcessor::ElementMaxHLevelPostProcessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters)
{
}

void
ElementMaxHLevelPostProcessor::initialize()
{
  _max_level = 0;
}

void
ElementMaxHLevelPostProcessor::execute()
{
  #ifdef LIBMESH_ENABLE_AMR
  _max_level = _current_elem->level();
  #endif
}

Real
ElementMaxHLevelPostProcessor::getValue() const
{
  return _max_level;
}

void
ElementMaxHLevelPostProcessor::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const ElementMaxHLevelPostProcessor &>(y);
  _max_level = std::max(_max_level, pps._max_level);
}

void
ElementMaxHLevelPostProcessor::finalize()
{
  gatherMax(_max_level);
}
