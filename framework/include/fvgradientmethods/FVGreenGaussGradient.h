//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVGradientMethod.h"

#include <utility>

class ComputeLinearFVGreenGaussGradientFaceThread;
class ElemInfo;
class FaceInfo;

/**
 * Green-Gauss cell-centered gradient method for linear finite-volume variables.
 */
class FVGreenGaussGradient : public FVGradientMethod
{
public:
  static InputParameters validParams();
  FVGreenGaussGradient(const InputParameters & params);

protected:
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /// Element-side and neighbor-side values contributed by an internal face.
  using InternalFaceValues = std::pair<Real, Real>;

  /**
   * Return the two Green-Gauss face values on an internal face.
   *
   * The first value is multiplied by the outward element surface vector. The second value is
   * multiplied by its opposite for the neighbor row.
   */
  virtual InternalFaceValues internalFaceValues(const FaceInfo & fi,
                                                const ElemInfo & elem_info,
                                                const ElemInfo & neighbor_info,
                                                Real elem_value,
                                                Real neighbor_value) const;

  friend class ComputeLinearFVGreenGaussGradientFaceThread;
};
