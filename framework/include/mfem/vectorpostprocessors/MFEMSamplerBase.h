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
 * Abstract base class for sampling MFEM quantities at points.
 *
 * Handles GSLIB point finding and coordinate output independently of the
 * quantity being sampled. Derived classes evaluate and output their quantity.
 */
class MFEMSamplerBase : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  /// Checks that all query points were found.
  void initialSetup() override;

  void initialize() override {}

  /// Outputs coordinates then delegates value output to finalizeValues().
  void finalize() override;

protected:
  /** Classification returned by GSLIB for a query point's location. */
  enum class PointLocationCode : unsigned int
  {
    INTERNAL = 0,
    BORDER = 1,
    NOT_FOUND = 2,
  };

  MFEMSamplerBase(const InputParameters & parameters,
                  const std::vector<Point> & points,
                  mfem::ParMesh & mesh);

  /// Copies interpolated values into the subclass VPP vectors.
  virtual void finalizeValues() = 0;

  /// Original query points used for point-location diagnostics.
  const std::vector<Point> _query_points;
  /// MFEM mesh on which the sampled quantity is defined.
  mfem::ParMesh & _mesh;
  /// GSLIB point finder used to locate and interpolate the query points.
  mfem::FindPointsGSLIB _finder;
  /// Ordering used to store the point coordinates in the MFEM vector.
  mfem::Ordering::Type _points_ordering;
  /// MFEM vector containing the query-point coordinates.
  mfem::Vector _points;
  /// Declared VPP output vectors for spatial coordinates ("x_0", "x_1", ...).
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_points;
};

#endif // MOOSE_MFEM_ENABLED
