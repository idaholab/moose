//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideAverage.h"

registerMooseObject("MooseApp", LayeredSideAverage);

InputParameters
LayeredSideAverage::validParams()
{
  InputParameters params = LayeredSideIntegral::validParams();
  params.addClassDescription("Computes side averages of a variable storing partial sums for the "
                             "specified number of intervals in a direction (x,y,z).");
  return params;
}

LayeredSideAverage::LayeredSideAverage(const InputParameters & parameters)
  : LayeredSideIntegral(parameters)
{
  _layer_volumes.resize(_num_layers);
}

void
LayeredSideAverage::initialize()
{
  LayeredSideIntegral::initialize();

  for (auto & vol : _layer_volumes)
    vol = 0.0;
}

void
LayeredSideAverage::execute()
{
  LayeredSideIntegral::execute();

  unsigned int layer = getLayer(_current_elem->vertex_average());
  _layer_volumes[layer] += _current_side_volume;
}

void
LayeredSideAverage::finalize()
{
  LayeredSideIntegral::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for (unsigned int i = 0; i < _layer_volumes.size(); i++)
    if (layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

void
LayeredSideAverage::threadJoin(const UserObject & y)
{
  LayeredSideIntegral::threadJoin(y);
  const LayeredSideAverage & lsa = static_cast<const LayeredSideAverage &>(y);
  for (unsigned int i = 0; i < _layer_volumes.size(); i++)
    if (lsa.layerHasValue(i))
      _layer_volumes[i] += lsa._layer_volumes[i];
}
