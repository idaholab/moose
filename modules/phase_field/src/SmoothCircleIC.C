#include "SmoothCircleIC.h"

template<>
InputParameters validParams<SmoothCircleIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("x1", "The x coordinate of the circle center");
  params.addRequiredParam<Real>("y1", "The y coordinate of the circle center");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the circle center");
  
  params.addRequiredParam<Real>("invalue", "The variable value inside the circle");
  params.addRequiredParam<Real>("outvalue", "The variable value outside the circle");
  params.addRequiredParam<Real>("radius", "The radius of a circle");
  return params;
}

SmoothCircleIC::SmoothCircleIC(const std::string & name,
                               MooseSystem & moose_system,
                               InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _x1(parameters.get<Real>("x1")),
   _y1(parameters.get<Real>("y1")),
   _z1(parameters.get<Real>("z1")),
   _invalue(parameters.get<Real>("invalue")),
   _outvalue(parameters.get<Real>("outvalue")),
   _radius(parameters.get<Real>("radius")),
   _center(_x1,_y1,_z1)
{}

Real
SmoothCircleIC::value(const Point & p)
{
  Real value = 0.0;
  
  Real rad = 0.0;
  
  for(unsigned int i=0; i<LIBMESH_DIM; i++) 
    rad += (p(i)-_center(i)) * (p(i)-_center(i));

  rad = sqrt(rad);
  
  if (rad <= _radius)
    value = _invalue;
  else if (rad < 1.5*_radius)
    value = _outvalue + (_invalue-_outvalue)*(1+cos((rad-_radius)/_radius*2.0*3.14159))/2.0;
  else
    value = _outvalue;

  return value;
  
}

  


  
