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

#include "LayeredSideAverage.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredSideAverage>()
{
  InputParameters params = validParams<LayeredSideIntegral>();

  return params;
}

LayeredSideAverage::LayeredSideAverage(const std::string & name, InputParameters parameters) :
    LayeredSideIntegral(name, parameters)
{
  _layer_volumes.resize(_num_layers);
}

void
LayeredSideAverage::initialize()
{
  LayeredSideIntegral::initialize();

  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    _layer_volumes[i] = 0.0;
}

void
LayeredSideAverage::execute()
{
  LayeredSideIntegral::execute();

  unsigned int layer = getLayer(_current_elem->centroid());
  _layer_volumes[layer] += _current_side_volume;
}

void
LayeredSideAverage::finalize()
{
  LayeredSideIntegral::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    if(layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

void
LayeredSideAverage::threadJoin(const UserObject & y)
{
  LayeredSideIntegral::threadJoin(y);
  const LayeredSideAverage & lsa = static_cast<const LayeredSideAverage &>(y);
  for(unsigned int i=0; i<_layer_volumes.size(); i++)
    if(lsa.layerHasValue(i))
      _layer_volumes[i] += lsa._layer_volumes[i];
}
