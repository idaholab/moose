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

#include "OrientedBoxMarker.h"

template<>
InputParameters validParams<OrientedBoxMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<RealVectorValue>("centre", "The centre (many people spell this 'center') of the box.");
  params.addRequiredParam<Real>("width", "The width of the box");
  params.addRequiredParam<Real>("length", "The length of the box");
  params.addRequiredParam<Real>("height", "The height of the box");
  params.addRequiredParam<RealVectorValue>("width_direction", "The direction along which the width is oriented.");
  params.addRequiredParam<RealVectorValue>("length_direction", "The direction along which the length is oriented (must be perpendicular to width_direction).");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>("inside", marker_states, "How to mark elements inside the box.");
  params.addRequiredParam<MooseEnum>("outside", marker_states, "How to mark elements outside the box.");

  params.addClassDescription("Marks inside and outside a box that can have arbitrary orientation and centre point");
  return params;
}


OrientedBoxMarker::OrientedBoxMarker(const std::string & name, InputParameters parameters) :
  Marker(name, parameters),
  // overall strategy is to create a bounding_box centred at the origin, and translate and rotate element centroids to this box
  _xmax(0.5*getParam<Real>("width")),
  _ymax(0.5*getParam<Real>("length")),
  _zmax(0.5*getParam<Real>("height")),
  _bottom_left(-_xmax, -_ymax, -_zmax),
  _top_right(_xmax, _ymax, _zmax),
  _bounding_box(_bottom_left, _top_right),

  _centre(getParam<RealVectorValue>("centre")),
  _w(getParam<RealVectorValue>("width_direction")),
  _l(getParam<RealVectorValue>("length_direction")),
  
  _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
  _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside"))
{
  // normalise vectors in readyness for insertion into rotation matrix
  Real len;

  len = std::pow(_w*_w, 0.5);
  if (len == 0)
    mooseError("Length of width_direction vector is zero in OrientedBoxMarker");
  _w /= len;

  len = std::pow(_l*_l, 0.5);
  if (len == 0)
    mooseError("Length of length_direction vector is zero in OrientedBoxMarker");
  _l /= len;

  if (_w*_l > 1E-10)
    mooseError("width_direction and length_direction are not perpendicular in OrientedBoxMarker");

  RealVectorValue h(_w(1)*_l(2)-_w(2)*_l(1), _w(2)*_l(0)-_w(0)*_l(2), _w(0)*_l(1)-_w(1)*_l(0)); // h=w cross l
  _rot_matrix(0, 0) = _w(0);
  _rot_matrix(0, 1) = _w(1);
  _rot_matrix(0, 2) = _w(2);
  _rot_matrix(1, 0) = _l(0);
  _rot_matrix(1, 1) = _l(1);
  _rot_matrix(1, 2) = _l(2);
  _rot_matrix(2, 0) = h(0);
  _rot_matrix(2, 1) = h(1);
  _rot_matrix(2, 2) = h(2);
    
}

Marker::MarkerValue
OrientedBoxMarker::computeElementMarker()
{
  RealVectorValue centroid = _current_elem->centroid();
  RealVectorValue transformed = _rot_matrix*(centroid - _centre);

  if(_bounding_box.contains_point(transformed))
    return _inside;

  return _outside;
}

