//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTrackerHaloRM.h"
#include "MooseMesh.h"
#include "Conversion.h"

registerMooseObject("PhaseFieldApp", GrainTrackerHaloRM);

template <>
InputParameters
validParams<GrainTrackerHaloRM>()
{
  InputParameters params = validParams<ElementPointNeighbors>();

  params.addRangeCheckedParam<unsigned short>(
      "halo_level",
      2,
      "halo_level>=1 & halo_level<=10",
      "The thickness of the halo surrounding each feature.");

  return params;
}

GrainTrackerHaloRM::GrainTrackerHaloRM(const InputParameters & parameters)
  : ElementPointNeighbors(parameters)
{
  // The grain tracker halo algorithm requires at least as many element neighbors as the required
  // halo level (See GrainTrackerInterface)
  _element_point_neighbor_layers = getParam<unsigned short>("halo_level");
}

std::string
GrainTrackerHaloRM::getInfo() const
{
  if (_point_coupling)
  {
    std::ostringstream oss;
    oss << "GrainTrackerHaloRM (" << _element_point_neighbor_layers << " layers)";
    return oss.str();
  }
  return "";
}
