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

#include "LayeredAverage.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredAverage>()
{
  InputParameters params = validParams<LayeredIntegral>();

  return params;
}

LayeredAverage::LayeredAverage(const std::string & name, InputParameters parameters) :
    LayeredIntegral(name, parameters)
{
  _layer_volumes.resize(_num_layers);
}

void
LayeredAverage::initialize()
{
  LayeredIntegral::initialize();

  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    _layer_volumes[i] = 0.0;
}

void
LayeredAverage::execute()
{
  LayeredIntegral::execute();

  unsigned int layer = getLayer(_current_elem->centroid());
  _layer_volumes[layer] += _current_elem_volume;
}

void
LayeredAverage::finalize()
{
  LayeredIntegral::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    if(layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

void
LayeredAverage::threadJoin(const UserObject & y)
{
  LayeredIntegral::threadJoin(y);
  const LayeredAverage & la = dynamic_cast<const LayeredAverage &>(y);
  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    _layer_volumes[i] += la._layer_volumes[i];
}
