#include "RndSmoothCircleIC.h"

template<>
InputParameters validParams<RndSmoothCircleIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("x1", "The x coordinate of the circle center");
  params.addRequiredParam<Real>("y1", "The y coordinate of the circle center");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the circle center");
  
  params.addRequiredParam<Real>("mx_invalue", "The max variable value inside the circle");
  params.addRequiredParam<Real>("mx_outvalue", "The max variable value outside the circle");
  params.addRequiredParam<Real>("mn_invalue", "The min variable value inside the circle");
  params.addRequiredParam<Real>("mn_outvalue", "The min variable value outside the circle");
  params.addRequiredParam<Real>("radius", "The radius of a circle");
  
  params.addParam<unsigned int>("seed", 12345, "Seed value for the random number generator");
  return params;
}

RndSmoothCircleIC::RndSmoothCircleIC(const std::string & name,
                               InputParameters parameters)
  :InitialCondition(name, parameters),
   _x1(parameters.get<Real>("x1")),
   _y1(parameters.get<Real>("y1")),
   _z1(parameters.get<Real>("z1")),
   _mx_invalue(parameters.get<Real>("mx_invalue")),
   _mn_invalue(parameters.get<Real>("mn_invalue")),
   _mx_outvalue(parameters.get<Real>("mx_outvalue")),
   _mn_outvalue(parameters.get<Real>("mn_outvalue")),
   _range_invalue(_mx_invalue - _mn_invalue),
   _range_outvalue(_mx_outvalue - _mn_outvalue),
   _radius(parameters.get<Real>("radius")),
   _center(_x1,_y1,_z1)
{
  mooseAssert(_range_invalue >= 0.0, "Inside Min > Max for RndSmoothCircleIC!");
  mooseAssert(_range_outvalue >= 0.0, "Outside Min > Max for RndSmoothCircleIC!");
  Moose::seed(getParam<unsigned int>("seed"));
}

Real
RndSmoothCircleIC::value(const Point & p)
{
  Real value = 0.0;
  
  Real rad = 0.0;
  
  for(unsigned int i=0; i<LIBMESH_DIM; i++) 
    rad += (p(i)-_center(i)) * (p(i)-_center(i));

  rad = sqrt(rad);
  
  //Random number between 0 and 1
  Real rand_num = Moose::rand();
  
  if (rad <= _radius)
    value = _mn_invalue + rand_num*(_range_invalue);
  else if (rad < 1.5*_radius)
  {
    Real av_invalue = (_mn_invalue + _mx_invalue)/2.0;
    Real av_outvalue = (_mn_outvalue + _mx_outvalue)/2.0;
    value = av_outvalue + (av_invalue-av_outvalue)*(1+cos((rad-_radius)/_radius*2.0*3.14159))/2.0;
  }
  else
    value = _mx_outvalue + rand_num*(_range_outvalue);

  return value;
  
}

  


  
