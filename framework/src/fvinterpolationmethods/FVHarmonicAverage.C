//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "fvinterpolationmethods/FVHarmonicAverage.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("MooseApp", FVHarmonicAverage);
registerMooseObjectAliased("MooseApp", FVHarmonicAverage, "HarmonicAverage");
registerMooseObjectAliased("MooseApp", FVHarmonicAverage, "harmonicAverage");

InputParameters
FVHarmonicAverage::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "Harmonic mean interpolation for finite-volume quantities using FaceInfo geometry weights.");
  return params;
}

FVHarmonicAverage::FVHarmonicAverage(const InputParameters & params) : FVInterpolationMethod(params)
{
  setFaceInterpolator(buildFaceInterpolator<FVHarmonicAverage>());
}

Real
FVHarmonicAverage::interpolate(const FaceInfo & face,
                               const Real elem_value,
                               const Real neighbor_value) const
{
  mooseAssert(face.neighborPtr(),
              "Harmonic interpolation is intended for internal faces with a neighbor.");
  // We will guard for the zeros below
  mooseAssert(
      (elem_value >= 0 && neighbor_value >= 0) || (elem_value <= 0 && neighbor_value <= 0),
      "Harmonic interpolation requires the element and neighbor values to have the same sign.");

  const Real gc = face.gC();
  const Real one_minus_gc = 1.0 - gc;

  // We guard against those nasty zeros here
  const auto safe = [](const Real value)
  {
    const Real eps = std::numeric_limits<Real>::min();
    return std::copysign(std::max(std::abs(value), eps), value);
  };

  const Real denom = gc / safe(elem_value) + one_minus_gc / safe(neighbor_value);
  return 1.0 / denom;
}
