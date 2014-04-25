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
  params.addParam<Real>("int_width", 0.0, "The interfacial width of the void surface.  Defaults to sharp interface");

  params.addParam<bool>("3D_spheres", true, "in 3D, whether the objects are spheres or columns");

  return params;
}

SmoothCircleIC::SmoothCircleIC(const std::string & name,
                               InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _x1(parameters.get<Real>("x1")),
    _y1(parameters.get<Real>("y1")),
    _z1(parameters.get<Real>("z1")),
    _invalue(parameters.get<Real>("invalue")),
    _outvalue(parameters.get<Real>("outvalue")),
    _radius(parameters.get<Real>("radius")),
    _int_width(parameters.get<Real>("int_width")),
    _3D_spheres(parameters.get<bool>("3D_spheres")),
    _center(_x1, _y1, _z1),
    _num_dim(_3D_spheres ? 3 : 2)
{
}

Real
SmoothCircleIC::value(const Point & p)
{
  Real value = 0.0;

  if (_num_dim < 1) //Loop dimension never initialized
    mooseError("Loop dimension in SmoothCircleIC was never initialized");

  //determine distance between current point and circle center
  Point p2 = p;
  Point center2 = _center;
  if (!_3D_spheres) //Create 3D cylinders instead of spheres
  {
    p2(2) = 0.0;
    center2(2) = 0.0;
  }

  //Set value for outside the circle, inside the circle, and on the smooth interface
  Real rad = _mesh.minPeriodicDistance(_var.number(), p2, center2);
  if (rad <= _radius - _int_width/2.0) //Inside circle
    value = _invalue;
  else if (rad < _radius + _int_width/2.0) //Smooth interface
  {
    Real int_pos = (rad - _radius + _int_width/2.0)/_int_width;
    value = _outvalue + (_invalue - _outvalue) * (1.0 + std::cos(int_pos * libMesh::pi)) / 2.0;
  }
  else //Outside circle
    value = _outvalue;

  return value;
}


RealGradient
SmoothCircleIC::gradient(const Point & p)
{
  Real DvalueDr = 0.0;

  //determine distance between current point and circle center
  Point p2 = p;
  Point center2 = _center;
  if (!_3D_spheres) //Create 3D cylinders instead of spheres
  {
    p2(2) = 0.0;
    center2(2) = 0.0;
  }

  //Determine derivative values over the smooth interface
  Real rad = _mesh.minPeriodicDistance(_var.number(), p2, center2);
  if (rad < _radius + _int_width/2.0 && rad > _radius - _int_width/2.0)
  {
    Real int_pos = (rad - _radius + _int_width / 2.0) / _int_width;
    Real Dint_posDr = 1.0 / _int_width;
    DvalueDr = Dint_posDr * (_invalue - _outvalue) * (-std::sin(int_pos * libMesh::pi) * libMesh::pi) / 2.0;
  }

  //Set gradient over the smooth interface
  if (rad != 0.0)
    return _mesh.minPeriodicVector(_var.number(), _center, p) * (DvalueDr / rad);
  else
    return Gradient(0.0, 0.0, 0.0);
}
