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

#include "LayeredSideIntegral.h"

// libmesh includes
#include "mesh_tools.h"

template<>
InputParameters validParams<LayeredSideIntegral>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();

  MooseEnum directions("x, y, z");

  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers.");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers.");

  return params;
}

LayeredSideIntegral::LayeredSideIntegral(const std::string & name, InputParameters parameters) :
    SideIntegralVariableUserObject(name, parameters),
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
LayeredSideIntegral::integralValue(Point p) const
{
  unsigned int layer = getLayer(p);

  return _layer_values[layer];
}

void
LayeredSideIntegral::initialize()
{
  SideIntegralVariableUserObject::initialize();

  for(unsigned int i=0; i<_layer_values.size(); i++)
    _layer_values[i] = 0.0;
}

void
LayeredSideIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->centroid());

  _layer_values[layer] += integral_value;
}

void
LayeredSideIntegral::finalize()
{
  gatherSum(_layer_values);
}

void
LayeredSideIntegral::threadJoin(const UserObject & y)
{
  SideIntegralVariableUserObject::threadJoin(y);
  const LayeredSideIntegral & li = dynamic_cast<const LayeredSideIntegral &>(y);
  for(unsigned int i=0; i<_layer_values.size(); i++)
    _layer_values[i] += li._layer_values[i];
}

unsigned int
LayeredSideIntegral::getLayer(Point p) const
{
  Real direction_x = p(_direction);

  unsigned int layer = std::floor(((direction_x - _direction_min) / (_direction_max - _direction_min)) * (Real)_num_layers);

  if(layer == _num_layers)
    layer -= 1;

  return layer;
}
