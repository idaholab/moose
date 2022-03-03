//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCFreezingIC.h"
#include "MooseRandom.h"

registerMooseObject("PhaseFieldApp", PFCFreezingIC);

InputParameters
PFCFreezingIC::validParams()
{
  InputParameters params = RandomICBase::validParams();
  params.addRequiredParam<Real>("x1",
                                "The x coordinate of the lower left-hand corner of the frozen box");
  params.addRequiredParam<Real>("y1",
                                "The y coordinate of the lower left-hand corner of the frozen box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addParam<Real>("min", 0.0, "Lower bound of the randomly generated values");
  params.addParam<Real>("max", 1.0, "Upper bound of the randomly generated values");
  params.addParam<Real>("inside", 1.0, "Value inside sinusoids");
  params.addParam<Real>("outside", 0.0, "Value outside sinusoids");

  params.addRequiredParam<Real>("lc", "The lattice constant off the crystal structure");

  MooseEnum crystal_structures("FCC BCC");
  params.addParam<MooseEnum>(
      "crystal_structure", crystal_structures, "The type of crystal structure");

  return params;
}

PFCFreezingIC::PFCFreezingIC(const InputParameters & parameters)
  : RandomICBase(parameters),
    _x1(getParam<Real>("x1")),
    _y1(getParam<Real>("y1")),
    _z1(getParam<Real>("z1")),
    _x2(getParam<Real>("x2")),
    _y2(getParam<Real>("y2")),
    _z2(getParam<Real>("z2")),
    _lc(getParam<Real>("lc")),
    _crystal_structure(getParam<MooseEnum>("crystal_structure")),
    _bottom_left(_x1, _y1, _z1),
    _top_right(_x2, _y2, _z2),
    _range(_top_right - _bottom_left),
    _min(getParam<Real>("min")),
    _max(getParam<Real>("max")),
    _val_range(_max - _min),
    _inside(getParam<Real>("inside")),
    _outside(getParam<Real>("outside"))
{
  _console << "MooseEnum? " << _crystal_structure << std::endl;

  for (const auto i : make_range(Moose::dim))
    if (_range(i) < 0.0)
      mooseError("x1, y1 or z1 is not less than x2, y2 or z2");

  if (_range(1) == 0.0)
    _icdim = 1;
  else if (_range(2) < 1.0e-10 * _range(0))
    _icdim = 2;
  else
    _icdim = 3;
}

Real
PFCFreezingIC::value(const Point & p)
{
  // If out of bounds, set random value
  for (const auto i : make_range(Moose::dim))
    if (p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return _min + _val_range * generateRandom();

  // If in bounds, set sinusoid IC to make atoms
  Real val = 0.0;
  if (_crystal_structure == "FCC")
  {
    // Note: this effectively (and now explicitly) returns 0.0 for FCC.
    return 0.0;

    for (unsigned int i = 0; i < _icdim; i++)
      val += std::cos((2.0 / _lc * p(i)) * libMesh::pi);
  }
  else
  {
    if (_icdim > 2)
    {
      for (unsigned int i = 0; i < _icdim; i++)
        // one mode approximation for initial condition
        val += (std::cos((2.0 / _lc * p(i % 3)) * libMesh::pi) *
                std::cos((2.0 / _lc * p((i + 1) % 3)) * libMesh::pi)) /
               4.0; // Doesn't work in 2D
    }
    else
    {
      for (unsigned int i = 0; i < _icdim; i++)
        val *= std::cos((2.0 / _lc * p(i)) * libMesh::pi); // 2D IC for 111 plane

      val = val / 2.0 + 0.5;
    }
  }

  Real amp = _inside - _outside;
  val = amp * val + _outside;

  return val;
}
