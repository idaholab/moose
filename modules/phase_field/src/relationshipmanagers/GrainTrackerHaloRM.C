/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GrainTrackerHaloRM.h"
#include "MooseMesh.h"
#include "Conversion.h"

template <>
InputParameters
validParams<GrainTrackerHaloRM>()
{
  InputParameters params = validParams<ElementSideNeighborLayers>();

  params.addRangeCheckedParam<unsigned short>(
      "halo_level",
      2,
      "halo_level>=1 & halo_level<=10",
      "The thickness of the halo surrounding each feature.");

  return params;
}

GrainTrackerHaloRM::GrainTrackerHaloRM(const InputParameters & parameters)
  : ElementSideNeighborLayers(parameters)
{
  // The grain tracker halo algorithm requires at least as many element neighbors as the required
  // halo level (See GrainTrackerInterface)
  _element_side_neighbor_layers = getParam<unsigned short>("halo_level");
}

std::string
GrainTrackerHaloRM::getInfo() const
{
  if (_default_coupling)
  {
    std::ostringstream oss;
    oss << "GrainTrackerHaloRM (" << _element_side_neighbor_layers << " layers)";
    return oss.str();
  }
  return "";
}
