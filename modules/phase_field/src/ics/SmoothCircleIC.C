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
  params.addParam<Real>("int_width",0.0,"The interfacial width of the void surface.  Defaults to sharp interface");

  params.addParam<bool>("3D_spheres",true,"in 3D, whether the objects are spheres or columns");

  return params;
}

SmoothCircleIC::SmoothCircleIC(const std::string & name,
                               InputParameters parameters)
  :InitialCondition(name, parameters),
   _x1(parameters.get<Real>("x1")),
   _y1(parameters.get<Real>("y1")),
   _z1(parameters.get<Real>("z1")),
   _invalue(parameters.get<Real>("invalue")),
   _outvalue(parameters.get<Real>("outvalue")),
   _radius(parameters.get<Real>("radius")),
   _int_width(parameters.get<Real>("int_width")),
   _3D_spheres(parameters.get<bool>("3D_spheres")),
   _center(_x1,_y1,_z1),
   _num_dim(_3D_spheres ? 3 : 2)
{
}

Real
SmoothCircleIC::value(const Point & p)
{
  Real value = 0.0;

  Real rad = 0.0;

  if (_num_dim < 1) //Loop dimension never initialized
    mooseError("Loop dimension in SmoothCircleIC was never initialized");

  for(unsigned int i=0; i<_num_dim; i++)
    rad += (p(i)-_center(i)) * (p(i)-_center(i));

  rad = sqrt(rad);

  if (rad <= _radius - _int_width/2.0)
    value = _invalue;
  else if (rad < _radius + _int_width/2.0)
  {
    Real int_pos = (rad - _radius + _int_width/2.0)/_int_width;
    value = _outvalue + (_invalue-_outvalue)*(1+cos(int_pos*libMesh::pi))/2.0;
  }
  else
    value = _outvalue;

  return value;

}


RealGradient
SmoothCircleIC::gradient(const Point & p)
{
  Real DvalueDr = 0.0;

  Real rad = 0.0;

  for(unsigned int i=0; i<_num_dim; i++)
    rad += (p(i)-_center(i)) * (p(i)-_center(i));

  rad = sqrt(rad);

  if (rad < _radius + _int_width/2.0 && rad > _radius - _int_width/2.0)
  {
    Real int_pos = (rad - _radius + _int_width/2.0)/_int_width;
    Real Dint_posDr = 1.0/_int_width;
    DvalueDr = Dint_posDr*(_invalue-_outvalue)*(-sin(int_pos*libMesh::pi)*libMesh::pi)/2.0;
  }

  if (rad != 0.0)
    return Gradient((p(0) - _center(0))*DvalueDr/rad,
                    (p(1) - _center(1))*DvalueDr/rad,
                    (p(2) - _center(2))*DvalueDr/rad);
  else
    return Gradient(0.0,0.0,0.0);

}





