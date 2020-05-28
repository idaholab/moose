//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetOlssonVortex.h"

registerMooseObject("LevelSetApp", LevelSetOlssonVortex);

InputParameters
LevelSetOlssonVortex::validParams()
{
  MooseEnum rtype("instantaneous=0 cosine=1", "instantaneous");

  InputParameters params = Function::validParams();
  params.addClassDescription(
      "A function for creating vortex velocity fields for level set equation benchmark problems.");
  params.addParam<MooseEnum>(
      "reverse_type", rtype, "The time of reversal to enforce (instantaneous or cosine).");
  params.addParam<Real>("reverse_time", 2, "Total time for complete vortex reversal.");
  return params;
}

LevelSetOlssonVortex::LevelSetOlssonVortex(const InputParameters & parameters)
  : Function(parameters),
    _reverse_time(getParam<Real>("reverse_time")),
    _reverse_type(getParam<MooseEnum>("reverse_type")),
    _pi(libMesh::pi)
{
}

RealVectorValue
LevelSetOlssonVortex::vectorValue(Real t, const Point & p) const
{
  // Compute the velocity field
  RealVectorValue output;
  output(0) = std::sin(_pi * p(0)) * std::sin(_pi * p(0)) * std::sin(2 * _pi * p(1));
  output(1) = -std::sin(_pi * p(1)) * std::sin(_pi * p(1)) * std::sin(2 * _pi * p(0));

  // Compute the coefficient used to reverse the flow
  Real reverse_coefficient = 1.0;
  if (_reverse_type == 0 && t > _reverse_time / 2.)
    reverse_coefficient = -1.0;
  else if (_reverse_type == 1)
    reverse_coefficient = std::cos(_pi * t / _reverse_time);
  return reverse_coefficient * output;
}
