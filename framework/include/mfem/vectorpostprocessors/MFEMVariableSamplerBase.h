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

  /// Name of the variable being sampled.
  const VariableName _var_name;

private:
  /// Return the continuity type of the sampled variable's finite element collection.
  virtual int getFESpaceContinuityType() const = 0;
};

#endif // MOOSE_MFEM_ENABLED
