//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * This functor material smooths a functor material property
 */
template <typename T>
class FunctorSmootherTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FunctorSmootherTempl(const InputParameters & parameters);

private:
  /// Incoming functor(s) names
  const std::vector<MooseFunctorName> _functors_in;

  /// Smoothed functor(s) names
  const std::vector<MooseFunctorName> _functors_out;

  /// This enum must match the enum in the input parameters
  enum SolveType
  {
    FACE_AVERAGE,
    LAYERED_AVERAGE,
    REMOVE_CHECKERBOARD
  };

  /// Smoothing technique to use
  const MooseEnum _smoothing_technique;
};

typedef FunctorSmootherTempl<Real> FunctorSmoother;
