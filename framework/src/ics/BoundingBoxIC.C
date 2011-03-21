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

#include "BoundingBoxIC.h"

template<>
InputParameters validParams<BoundingBoxIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addParam<Real>("inside", 0.0, "The value of the variable inside the box");
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the box");
  return params;
}

BoundingBoxIC::BoundingBoxIC(const std::string & name, InputParameters parameters) :
  InitialCondition(name, parameters),
  _x1(getParam<Real>("x1")),
  _y1(getParam<Real>("y1")),
  _z1(getParam<Real>("z1")),
  _x2(getParam<Real>("x2")),
  _y2(getParam<Real>("y2")),
  _z2(getParam<Real>("z2")),
  _inside(getParam<Real>("inside")),
  _outside(getParam<Real>("outside")),
  _bottom_left(_x1,_y1,_z1),
  _top_right(_x2,_y2,_z2)
{
}

Real
BoundingBoxIC::value(const Point & p)
{
  for (unsigned int i=0; i<LIBMESH_DIM; i++)
    if (p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return _outside;

  return _inside;
}
  
