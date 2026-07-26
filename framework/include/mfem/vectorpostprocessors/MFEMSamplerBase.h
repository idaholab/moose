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

#include "MFEMVectorPostprocessor.h"

/**
 * Abstract base class for MFEM point/line value samplers.
 *
 * Handles all GSLIB point-finding machinery and coordinate output that is
 * shared between real-valued (MFEMValueSamplerBase) and complex-valued
 * (MFEMComplexValueSamplerBase) samplers. Derived classes only need to
 * implement execute() (to interpolate their variable) and finalizeValues()
 * (to copy interpolated results into their declared VPP vectors).
 */
class MFEMSamplerBase : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  /// Checks that all query points were found and warns about discontinuous boundary values.
  void initialSetup() override;

  void initialize() override {}

  /// Outputs coordinates then delegates value output to finalizeValues().
  void finalize() override;

protected:
  MFEMSamplerBase(const InputParameters & parameters, const std::vector<Point> & points);

  /// Copies interpolated variable values into the subclass VPP vectors.
  virtual void finalizeValues() = 0;

  /// Name of the variable being sampled (used by derived classes).
  const VariableName _var_name;
  /// Original query points used for point-location diagnostics.
  const std::vector<Point> _query_points;
  /// MFEM mesh on which the sampled variable is defined.
  mfem::ParMesh & _mesh;
  /// GSLIB point finder used to locate and interpolate the query points.
  mfem::FindPointsGSLIB _finder;
  /// Ordering used to store the point coordinates in the MFEM vector.
  mfem::Ordering::Type _points_ordering;
  /// MFEM vector containing the query-point coordinates.
  mfem::Vector _points;
  /// Declared VPP output vectors for spatial coordinates ("x_0", "x_1", ...).
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_points;

private:
  /// Whether the sampled variable's finite element space is discontinuous at element boundaries.
  virtual bool isFESpaceDiscontinuous() const = 0;
};

#endif // MOOSE_MFEM_ENABLED
