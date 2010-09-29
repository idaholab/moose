#include "GradientBoxIC.h"

template<>
InputParameters validParams<GradientBoxIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addRequiredParam<Real>("mx_value", "The max value of the variable");
  params.addRequiredParam<Real>("mn_value", "The min value of the variable");

  params.addRequiredParam<int>("grad_direction","An integer from 0 to 2 defining the direction of the linear gradient");
  params.addRequiredParam<int>("grad_sign","An integer with value 1 or -1 defining sign of the gradient slope");
  
  return params;
}

GradientBoxIC::GradientBoxIC(const std::string & name,
                             MooseSystem & moose_system,
                             InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _x1(parameters.get<Real>("x1")),
   _y1(parameters.get<Real>("y1")),
   _z1(parameters.get<Real>("z1")),
   _x2(parameters.get<Real>("x2")),
   _y2(parameters.get<Real>("y2")),
   _z2(parameters.get<Real>("z2")),
   _mx_value(parameters.get<Real>("mx_value")),
   _mn_value(parameters.get<Real>("mn_value")),
   _grad_direction(parameters.get<int>("grad_direction")),
   _grad_sign(parameters.get<int>("grad_sign")),
   _range(_mx_value - _mn_value),
   _bottom_left(_x1,_y1,_z1),
   _top_right(_x2,_y2,_z2)
{
  mooseAssert(_range > 0.0, "Min > Max for gradient!");
}

Real
GradientBoxIC::value(const Point & p)
{
  //Check to see if out of bounds
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    if(p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return (_mx_value+_mn_value)/2.0;

  //If not out of bounds
  if (_grad_sign > 0)
    return _mn_value + _range*p(_grad_direction)/(_top_right(_grad_direction) - _bottom_left(_grad_direction));
  else 
    return _mx_value - _range*p(_grad_direction)/(_top_right(_grad_direction) - _bottom_left(_grad_direction));
  
}

  


  
