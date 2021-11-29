//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredAverage.h"

registerMooseObject("MooseApp", LayeredAverage);

InputParameters
LayeredAverage::validParams()
{
  InputParameters params = LayeredIntegral::validParams();
  params.addClassDescription("Computes averages of variables over layers");

  return params;
}

LayeredAverage::LayeredAverage(const InputParameters & parameters) : LayeredIntegral(parameters)
{
  _layer_volumes.resize(_num_layers);
}

void
LayeredAverage::initialize()
{
  LayeredIntegral::initialize();

  for (auto & vol : _layer_volumes)
    vol = 0.0;
}

void
LayeredAverage::execute()
{
  LayeredIntegral::execute();

  unsigned int layer = getLayer(_current_elem->vertex_average());
  _layer_volumes[layer] += _current_elem_volume;
}

void
LayeredAverage::finalize()
{
  LayeredIntegral::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for (unsigned int i = 0; i < _layer_volumes.size(); i++)
    if (layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

void
LayeredAverage::threadJoin(const UserObject & y)
{
  LayeredIntegral::threadJoin(y);
  const LayeredAverage & la = static_cast<const LayeredAverage &>(y);
  for (unsigned int i = 0; i < _layer_volumes.size(); i++)
    _layer_volumes[i] += la._layer_volumes[i];
}
