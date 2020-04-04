//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothSuperellipsoidBaseIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

InputParameters
SmoothSuperellipsoidBaseIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<Real>("invalue", "The variable value inside the superellipsoid");
  params.addRequiredParam<Real>("outvalue", "The variable value outside the superellipsoid");
  params.addParam<Real>(
      "nestedvalue",
      "The variable value for nested particles inside the superellipsoid in inverse configuration");
  params.addParam<Real>(
      "int_width", 0.0, "The interfacial width of the void surface.  Defaults to sharp interface");
  params.addParam<bool>("zero_gradient",
                        false,
                        "Set the gradient DOFs to zero. This can avoid "
                        "numerical problems with higher order shape "
                        "functions.");
  params.addParam<unsigned int>("rand_seed", 12345, "Seed value for the random number generator");
  return params;
}

SmoothSuperellipsoidBaseIC::SmoothSuperellipsoidBaseIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _invalue(parameters.get<Real>("invalue")),
    _outvalue(parameters.get<Real>("outvalue")),
    _nestedvalue(isParamValid("nestedvalue") ? parameters.get<Real>("nestedvalue")
                                             : parameters.get<Real>("outvalue")),
    _int_width(parameters.get<Real>("int_width")),
    _zero_gradient(parameters.get<bool>("zero_gradient"))
{
  _random.seed(_tid, getParam<unsigned int>("rand_seed"));
}

void
SmoothSuperellipsoidBaseIC::initialSetup()
{
  // Compute centers, semiaxes, exponents, and initialize vector sizes
  computeSuperellipsoidSemiaxes();
  computeSuperellipsoidExponents();
  computeSuperellipsoidCenters();

  if (_centers.size() != _as.size())
    mooseError("_center and semiaxis _as vectors are not the same size in the Superellipsoid IC");
  if (_centers.size() != _bs.size())
    mooseError("_center and semiaxis _bs vectors are not the same size in the Superellipsoid IC");
  if (_centers.size() != _cs.size())
    mooseError("_center and semiaxis _cs vectors are not the same size in the Superellipsoid IC");
  if (_centers.size() != _ns.size())
    mooseError("_center and exponent _ns vectors are not the same size in the Superellipsoid IC");

  if (_centers.size() < 1)
    mooseError("_centers, _as, _bs, _cs, and _ns were not initialized in the Superellipsoid IC");
}

Real
SmoothSuperellipsoidBaseIC::value(const Point & p)
{
  Real value = _outvalue;
  Real val2 = 0.0;

  for (unsigned int ellip = 0; ellip < _centers.size() && value != _invalue; ++ellip)
  {
    val2 = computeSuperellipsoidValue(
        p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ((val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue))
      value = val2;
  }

  return value;
}

RealGradient
SmoothSuperellipsoidBaseIC::gradient(const Point & p)
{
  if (_zero_gradient)
    return 0.0;

  RealGradient gradient = 0.0;
  Real value = _outvalue;
  Real val2 = 0.0;

  for (unsigned int ellip = 0; ellip < _centers.size(); ++ellip)
  {
    val2 = computeSuperellipsoidValue(
        p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ((val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue))
    {
      value = val2;
      gradient = computeSuperellipsoidGradient(
          p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    }
  }

  return gradient;
}

Real
SmoothSuperellipsoidBaseIC::computeSuperellipsoidValue(
    const Point & p, const Point & center, Real a, Real b, Real c, Real n)
{
  Point l_center = center;
  Point l_p = p;
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  // When dist is 0 we are exactly at the center of the superellipsoid so return _invalue
  // Handle this case independently because we cannot calculate polar angles at this point
  if (dist == 0.0)
    return _invalue;

  // Compute the distance r from the center of the superellipsoid to its outside edge
  // along the vector from the center to the current point
  // This uses the equation for a superellipse in polar coordinates and substitutes
  // distances for sin, cos functions
  Point dist_vec = _mesh.minPeriodicVector(_var.number(), center, p);

  // First calculate rmn = r^(-n), replacing sin, cos functions with distances
  Real rmn = (std::pow(std::abs(dist_vec(0) / dist / a), n) +
              std::pow(std::abs(dist_vec(1) / dist / b), n) +
              std::pow(std::abs(dist_vec(2) / dist / c), n));
  // Then calculate r from rmn
  Real r = std::pow(rmn, (-1.0 / n));

  Real value = _outvalue; // Outside superellipsoid

  if (dist <= r - _int_width / 2.0) // Inside superellipsoid
    value = _invalue;
  else if (dist < r + _int_width / 2.0) // Smooth interface
  {
    Real int_pos = (dist - r + _int_width / 2.0) / _int_width;
    value = _outvalue + (_invalue - _outvalue) * (1.0 + std::cos(int_pos * libMesh::pi)) / 2.0;
  }

  return value;
}

// Following function does the same as computeSuperellipsoidValue but reverses invalue and outvalue
Real
SmoothSuperellipsoidBaseIC::computeSuperellipsoidInverseValue(
    const Point & p, const Point & center, Real a, Real b, Real c, Real n)
{
  Point l_center = center;
  Point l_p = p;
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  // When dist is 0 we are exactly at the center of the superellipsoid so return _invalue
  // Handle this case independently because we cannot calculate polar angles at this point
  if (dist == 0.0)
    return _nestedvalue;

  // Compute the distance r from the center of the superellipsoid to its outside edge
  // along the vector from the center to the current point
  // This uses the equation for a superellipse in polar coordinates and substitutes
  // distances for sin, cos functions
  Point dist_vec = _mesh.minPeriodicVector(_var.number(), center, p);

  // First calculate rmn = r^(-n), replacing sin, cos functions with distances
  Real rmn = (std::pow(std::abs(dist_vec(0) / dist / a), n) +
              std::pow(std::abs(dist_vec(1) / dist / b), n) +
              std::pow(std::abs(dist_vec(2) / dist / c), n));
  // Then calculate r from rmn
  Real r = std::pow(rmn, (-1.0 / n));

  Real value = _invalue;

  if (dist <= r - _int_width / 2.0) // Reversing inside and outside values
    value = _nestedvalue;
  else if (dist < r + _int_width / 2.0) // Smooth interface
  {
    Real int_pos = (dist - r + _int_width / 2.0) / _int_width;
    value = _invalue + (_nestedvalue - _invalue) * (1.0 + std::cos(int_pos * libMesh::pi)) / 2.0;
  }

  return value;
}

RealGradient
SmoothSuperellipsoidBaseIC::computeSuperellipsoidGradient(
    const Point & p, const Point & center, Real a, Real b, Real c, Real n)
{
  Point l_center = center;
  Point l_p = p;
  // Compute the distance between the current point and the center
  Real dist = _mesh.minPeriodicDistance(_var.number(), l_p, l_center);

  // When dist is 0 we are exactly at the center of the superellipsoid so return 0
  // Handle this case independently because we cannot calculate polar angles at this point
  if (dist == 0.0)
    return 0.0;

  // Compute the distance r from the center of the superellipsoid to its outside edge
  // along the vector from the center to the current point
  // This uses the equation for a superellipse in polar coordinates and substitutes
  // distances for sin, cos functions
  Point dist_vec = _mesh.minPeriodicVector(_var.number(), center, p);
  // First calculate rmn = r^(-n)
  Real rmn = (std::pow(std::abs(dist_vec(0) / dist / a), n) +
              std::pow(std::abs(dist_vec(1) / dist / b), n) +
              std::pow(std::abs(dist_vec(2) / dist / c), n));
  // Then calculate r from rmn
  Real r = std::pow(rmn, (-1.0 / n));

  Real DvalueDr = 0.0;

  if (dist < r + _int_width / 2.0 && dist > r - _int_width / 2.0) // in interfacial region
  {
    Real int_pos = (dist - r + _int_width / 2.0) / _int_width;
    Real Dint_posDr = 1.0 / _int_width;
    DvalueDr = Dint_posDr * (_invalue - _outvalue) *
               (-std::sin(int_pos * libMesh::pi) * libMesh::pi) / 2.0;
  }

  return dist_vec * (DvalueDr / dist);
}
