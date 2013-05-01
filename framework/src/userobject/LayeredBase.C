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

#include "LayeredBase.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredBase>()
{
  InputParameters params = emptyInputParameters();
  MooseEnum directions("x, y, z");

  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers.");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers.");
  params.addParam<bool>("interpolate", false, "Whether to interpolate (true) between layers or to use constant values (false).");

  return params;
}

LayeredBase::LayeredBase(const std::string & name, InputParameters parameters) :
    _layered_base_name(name),
    _layered_base_params(parameters),
    _direction_enum(parameters.get<MooseEnum>("direction")),
    _direction(_direction_enum),
    _num_layers(parameters.get<unsigned int>("num_layers")),
    _interpolate(parameters.get<bool>("interpolate")),
    _layered_base_subproblem(*parameters.get<SubProblem *>("_subproblem"))
{
  MeshTools::BoundingBox bounding_box = MeshTools::bounding_box(_layered_base_subproblem.mesh());
  _layer_values.resize(_num_layers);
  _layer_has_value.resize(_num_layers);

  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);
}

Real
LayeredBase::integralValue(Point p) const
{
  unsigned int layer = getLayer(p);

  int higher_layer = -1;
  int lower_layer = -1;

  for(int i=layer; i<_layer_values.size(); i++)
  {
    if(_layer_has_value[i])
    {
      higher_layer = i;
      break;
    }
  }

  for(int i=layer-1; i>=0; i--)
  {
    if(_layer_has_value[i])
    {
      lower_layer = i;
      break;
    }
  }

  if(higher_layer == -1 && lower_layer == -1)
    return 0; // TODO: We could error here but there are startup dependency problems

  if(higher_layer == -1) // Didn't find a higher layer
    return _layer_values[lower_layer];

  if(lower_layer == -1) // Didn't find a lower layer
    return _layer_values[higher_layer];

  if (!_interpolate) // constant within a layer
  {
    if (higher_layer == layer) // constant in a layer
      return _layer_values[higher_layer];

    return (_layer_values[higher_layer] + _layer_values[lower_layer]) / 2;
  }

  // Interpolate values

  Real layer_length = (_direction_max-_direction_min)/_num_layers;
  Real lower_coor = _direction_min;
  Real lower_value = 0;
  if (lower_layer != -1)
  {
    lower_coor += (lower_layer+1) * layer_length;
    lower_value = _layer_values[lower_layer];
  }

  // Interpolate between the two points
  Real higher_value = _layer_values[higher_layer];

  // Linear interpolation
  return lower_value + (higher_value - lower_value) * ( p(_direction) - lower_coor ) / layer_length;
}


Real
LayeredBase::getLayerValue(unsigned int layer) const
{
  if (layer >= _layer_values.size())
    mooseError("Layer '" << layer << "' not found in '" << _layered_base_name << "'.");
  return _layer_values[layer];
}

void
LayeredBase::initialize()
{
  for(unsigned int i=0; i<_layer_values.size(); i++)
  {
    _layer_values[i] = 0.0;
    _layer_has_value[i] = false;
  }
}

void
LayeredBase::finalize()
{
  Parallel::sum(_layer_values);
  Parallel::max(_layer_has_value);
}

void
LayeredBase::threadJoin(const UserObject & y)
{
  const LayeredBase & lb = dynamic_cast<const LayeredBase &>(y);
  for(unsigned int i=0; i<_layer_values.size(); i++)
    if(lb.layerHasValue(i))
      setLayerValue(i, getLayerValue(i) + lb._layer_values[i]);
}

unsigned int
LayeredBase::getLayer(Point p) const
{
  Real direction_x = p(_direction);

  if(direction_x < _direction_min)
    return 0;

  unsigned int layer = std::floor(((direction_x - _direction_min) / (_direction_max - _direction_min)) * (Real)_num_layers);

  if(layer >= _num_layers)
    layer = _num_layers-1;

  return layer;
}

void
LayeredBase::setLayerValue(unsigned int layer, Real value)
{
  _layer_values[layer] = value;
  _layer_has_value[layer] = true;
}
