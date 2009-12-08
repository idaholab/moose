#include "BoundingBoxIC.h"

template<>
InputParameters validParams<BoundingBoxIC>()
{
  InputParameters params;
  params.addParam<Real>("x1", 0.0, "The x coordinate of the lower left-hand corner of the box", true);
  params.addParam<Real>("y1", 0.0, "The y coordinate of the lower left-hand corner of the box", true);
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box", false);

  params.addParam<Real>("x2", 0.0, "The x coordinate of the upper right-hand corner of the box", true);
  params.addParam<Real>("y2", 0.0, "The y coordinate of the upper right-hand corner of the box", true);
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box", false);

  params.addParam<Real>("inside", 0.0, "The value of the variable inside the box", false);
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the box", false);
  return params;
}

BoundingBoxIC::BoundingBoxIC(std::string name,
                       InputParameters parameters,
                       std::string var_name)
  :InitialCondition(name,parameters,var_name),
   _x1(parameters.get<Real>("x1")),
   _y1(parameters.get<Real>("y1")),
   _z1(parameters.get<Real>("z1")),
   _x2(parameters.get<Real>("x2")),
   _y2(parameters.get<Real>("y2")),
   _z2(parameters.get<Real>("z2")),
   _inside(parameters.get<Real>("inside")),
   _outside(parameters.get<Real>("outside")),
   _bottom_left(_x1,_y1,_z1),
   _top_right(_x2,_y2,_z2)
{}

Real
BoundingBoxIC::value(const Point & p)
{
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    if(p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return _outside;

  return _inside;
}

  


  
