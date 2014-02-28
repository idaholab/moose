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

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */

#include "OrientedBoxMarker.h"

template<>
InputParameters validParams<OrientedBoxMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<RealVectorValue>("center", "The center (many people spell this 'center') of the box.");
  params.addRequiredParam<Real>("width", "The width of the box");
  params.addRequiredParam<Real>("length", "The length of the box");
  params.addRequiredParam<Real>("height", "The height of the box");
  params.addRequiredParam<RealVectorValue>("width_direction", "The direction along which the width is oriented.");
  params.addRequiredParam<RealVectorValue>("length_direction", "The direction along which the length is oriented (must be perpendicular to width_direction).");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>("inside", marker_states, "How to mark elements inside the box.");
  params.addRequiredParam<MooseEnum>("outside", marker_states, "How to mark elements outside the box.");

  params.addClassDescription("Marks inside and outside a box that can have arbitrary orientation and center point");
  return params;
}


/**
 * This constructor does most of the work.
 * The overall strategy is to create a box of the
 * required size which is centered at the origin, with
 * the width along the x axis
 * the length along the y axis
 * the height along the z axis
 * Then create the transformation from real space
 * into this box, which is:
 * a translation from center to the origin, then
 * a rotation from the oriented box frame to this frame
 */
OrientedBoxMarker::OrientedBoxMarker(const std::string & name, InputParameters parameters) :
  Marker(name, parameters),
  /*
   * Create the box centered at the origin
   */
  _xmax(0.5*getParam<Real>("width")),
  _ymax(0.5*getParam<Real>("length")),
  _zmax(0.5*getParam<Real>("height")),
  _bottom_left(-_xmax, -_ymax, -_zmax),
  _top_right(_xmax, _ymax, _zmax),
  _bounding_box(_bottom_left, _top_right),

  /*
   * Create the translation, which is just center
   */
  _center(getParam<RealVectorValue>("center")),

  _w(getParam<RealVectorValue>("width_direction")),
  _l(getParam<RealVectorValue>("length_direction")),

  _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
  _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside"))
{
  /*
   * now create the rotation matrix that rotates the oriented
   * box's width direction to "x", its length direction to "y"
   * and its height direction to "z"
   */
  Real len;

  /*
   * Normalise the width and length directions in readiness for
   * insertion into the rotation matrix
   */
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
  /*
   * The rotation matrix!
   */
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

/**
 * Marks elements inside and outside the box
 */
Marker::MarkerValue
OrientedBoxMarker::computeElementMarker()
{
  RealVectorValue centroid = _current_elem->centroid();
  /*
   * This next line is the key step:
   * translate to the origin, and then rotate
   */
  RealVectorValue transformed = _rot_matrix*(centroid - _center);

  if(_bounding_box.contains_point(transformed))
    return _inside;

  return _outside;
}

