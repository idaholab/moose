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

#include "LayeredIntegral.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredIntegral>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();
  params.set<std::string>("built_by_action") = "add_user_object";

  MooseEnum directions("x, y, z");

  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers.");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers.");

  return params;
}

LayeredIntegral::LayeredIntegral(const std::string & name, InputParameters parameters) :
    ElementIntegralVariableUserObject(name, parameters),
    _direction_enum(parameters.get<MooseEnum>("direction")),
    _direction(_direction_enum),
    _num_layers(parameters.get<unsigned int>("num_layers"))
{
  MeshTools::BoundingBox bounding_box = MeshTools::bounding_box(_subproblem.mesh());
  _layer_values.resize(_num_layers);

  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);
}

Real
LayeredIntegral::integralValue(Point p) const
{
  unsigned int layer = getLayer(p);
  return _layer_values[layer];
}

Real
LayeredIntegral::getLayerValue(unsigned int layer) const
{
  if (layer >= _layer_values.size())
    mooseError("Layer '" << layer << "' not found in '" << name() << "'.");
  return _layer_values[layer];
}

void
LayeredIntegral::initialize()
{
  ElementIntegralVariableUserObject::initialize();

  for(unsigned int i=0; i<_layer_values.size(); i++)
    _layer_values[i] = 0.0;
}

void
LayeredIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->centroid());

  _layer_values[layer] += integral_value;
}

void
LayeredIntegral::finalize()
{
  gatherSum(_layer_values);
}

void
LayeredIntegral::threadJoin(const UserObject & y)
{
  ElementIntegralVariableUserObject::threadJoin(y);
  const LayeredIntegral & li = dynamic_cast<const LayeredIntegral &>(y);
  for(unsigned int i=0; i<_layer_values.size(); i++)
    _layer_values[i] += li._layer_values[i];
}

unsigned int
LayeredIntegral::getLayer(Point p) const
{
  Real direction_x = p(_direction);

  unsigned int layer = std::floor(((direction_x - _direction_min) / (_direction_max - _direction_min)) * (Real)_num_layers);

  if(layer == _num_layers)
    layer -= 1;

  return layer;
}
