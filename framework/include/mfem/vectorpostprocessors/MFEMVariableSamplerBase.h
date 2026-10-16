//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMSamplerBase.h"

namespace Moose::MFEM
{
std::vector<Point>
generateLinePoints(const Point & start_point, const Point & end_point, unsigned int num_points);
}

/**
 * Base class for sampling real or complex MFEM variables at points.
 *
 * Resolves the mesh belonging to the sampled variable, configures interpolation
 * at element boundaries, and diagnoses sampling finite element spaces that are not pointwise
 * continuous there.
 */
class MFEMVariableSamplerBase : public MFEMSamplerBase
{
public:
  static InputParameters validParams();

  /// Checks point locations and warns when sampled components may be discontinuous at boundaries.
  void initialSetup() override;

protected:
  MFEMVariableSamplerBase(const InputParameters & parameters, const std::vector<Point> & points);

  /// Names of the variables being sampled.
  const std::vector<VariableName> _var_names;

private:
  /// Return the continuity type of the i-th sampled variable's finite element collection.
  virtual int getFESpaceContinuityType(size_t i_var) const = 0;
};

#endif // MOOSE_MFEM_ENABLED
