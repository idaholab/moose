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

  void initialize() override {}

  /// Outputs coordinates then delegates value output to finalizeValues().
  void finalize() override;

  /**
   * Returns the flat index into an mfem::Vector that stores field values with a
   * given MFEM dimension ordering, for a given component and point index.
   */
  static size_t mfemIndex(size_t i_dim,
                          size_t i_point,
                          size_t num_dims,
                          size_t num_points,
                          mfem::Ordering::Type ordering);

protected:
  MFEMSamplerBase(const InputParameters & parameters, const std::vector<Point> & points);

  /// Copies interpolated variable values into the subclass VPP vectors.
  virtual void finalizeValues() = 0;

  /// Name of the variable being sampled (used by derived classes).
  const VariableName _var_name;
  mfem::ParMesh & _mesh;
  mfem::FindPointsGSLIB _finder;
  mfem::Ordering::Type _points_ordering;
  mfem::Vector _points;
  /// Declared VPP output vectors for spatial coordinates ("x_0", "x_1", ...).
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_points;
};

#endif // MOOSE_MFEM_ENABLED
