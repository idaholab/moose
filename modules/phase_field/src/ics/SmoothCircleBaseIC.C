/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SmoothCircleBaseIC.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<SmoothCircleBaseIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("invalue", "The variable value inside the circle");
  params.addRequiredParam<Real>("outvalue", "The variable value outside the circle");
  params.addParam<Real>(
      "int_width", 0.0, "The interfacial width of the void surface.  Defaults to sharp interface");
  params.addParam<bool>("3D_spheres", true, "in 3D, whether the objects are spheres or columns");
  params.addParam<bool>("zero_gradient",
                        false,
                        "Set the gradient DOFs to zero. This can avoid "
                        "numerical problems with higher order shape "
                        "functions and overlapping circles.");
  params.addParam<unsigned int>("rand_seed", 12345, "Seed value for the random number generator");
  return params;
}

SmoothCircleBaseIC::SmoothCircleBaseIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _invalue(parameters.get<Real>("invalue")),
    _outvalue(parameters.get<Real>("outvalue")),
    _int_width(parameters.get<Real>("int_width")),
    _3D_spheres(parameters.get<bool>("3D_spheres")),
    _zero_gradient(parameters.get<bool>("zero_gradient")),
    _num_dim(_3D_spheres ? 3 : 2)
{
  _random.seed(_tid, getParam<unsigned int>("rand_seed"));
}

void
SmoothCircleBaseIC::initialSetup()
{
  // Compute radii and centers and initialize vector sizes
  computeCircleRadii();
  computeCircleCenters();

  if (_centers.size() != _radii.size())
    mooseError("_center and _radii vectors are not the same size in the Circle IC");

  if (_centers.size() < 1)
    mooseError("_center and _radii were not initialized in the Circle IC");
}

Real
SmoothCircleBaseIC::value(const Point & p)
{
  Real value = _outvalue;
  Real val2 = 0.0;

  for (unsigned int circ = 0; circ < _centers.size() && value != _invalue; ++circ)
  {
    val2 = computeCircleValue(p, _centers[circ], _radii[circ]);
    if ((val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue))
      value = val2;
  }

  return value;
}

RealGradient
SmoothCircleBaseIC::gradient(const Point & p)
{
  if (_zero_gradient)
    return 0.0;

  RealGradient gradient = 0.0;
  Real value = _outvalue;
  Real val2 = 0.0;

  for (unsigned int circ = 0; circ < _centers.size(); ++circ)
  {
    val2 = computeCircleValue(p, _centers[circ], _radii[circ]);
    if ((val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue))
    {
      value = val2;
      gradient = computeCircleGradient(p, _centers[circ], _radii[circ]);
    }
  }

  return gradient;
}

Real
SmoothCircleBaseIC::computeCircleValue(const Point & p, const Point & center, const Real & radius)
{
  Point l_center = center;
  Point l_p = p;
  if (!_3D_spheres) // Create 3D cylinders instead of spheres
  {
    l_p(2) = 0.0;
    l_center(2) = 0.0;
  }
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  // Return value
  Real value = _outvalue; // Outside circle

  if (dist <= radius - _int_width / 2.0) // Inside circle
    value = _invalue;
  else if (dist < radius + _int_width / 2.0) // Smooth interface
  {
    Real int_pos = (dist - radius + _int_width / 2.0) / _int_width;
    value = _outvalue + (_invalue - _outvalue) * (1.0 + std::cos(int_pos * libMesh::pi)) / 2.0;
  }

  return value;
}

RealGradient
SmoothCircleBaseIC::computeCircleGradient(const Point & p,
                                          const Point & center,
                                          const Real & radius)
{
  Point l_center = center;
  Point l_p = p;
  if (!_3D_spheres) // Create 3D cylinders instead of spheres
  {
    l_p(2) = 0.0;
    l_center(2) = 0.0;
  }
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  Real DvalueDr = 0.0;

  if (dist < radius + _int_width / 2.0 && dist > radius - _int_width / 2.0)
  {
    Real int_pos = (dist - radius + _int_width / 2.0) / _int_width;
    Real Dint_posDr = 1.0 / _int_width;
    DvalueDr = Dint_posDr * (_invalue - _outvalue) *
               (-std::sin(int_pos * libMesh::pi) * libMesh::pi) / 2.0;
  }

  // Set gradient over the smooth interface
  if (dist != 0.0)
    return _mesh.minPeriodicVector(_var.number(), center, p) * (DvalueDr / dist);
  else
    return 0.0;
}
