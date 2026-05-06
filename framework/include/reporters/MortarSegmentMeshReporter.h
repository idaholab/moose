//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 * Reports mortar segment mesh statistics (element counts and volume statistics) for all mortar
 * interfaces in the problem. One entry per primary-secondary subdomain pair is appended to each
 * output vector, in the order the interfaces are iterated.
 */
class MortarSegmentMeshReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  MortarSegmentMeshReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

private:
  /// Whether to report statistics for the displaced mortar interfaces
  const bool _on_displaced;

  /// Element count in the secondary lower-dimensional subdomain, one entry per mortar interface
  std::vector<unsigned int> & _secondary_lower_n_elems;
  /// Maximum element volume in the secondary lower-dimensional subdomain
  std::vector<Real> & _secondary_lower_max_volume;
  /// Minimum element volume in the secondary lower-dimensional subdomain
  std::vector<Real> & _secondary_lower_min_volume;
  /// Median element volume in the secondary lower-dimensional subdomain
  std::vector<Real> & _secondary_lower_median_volume;

  /// Element count in the primary lower-dimensional subdomain, one entry per mortar interface
  std::vector<unsigned int> & _primary_lower_n_elems;
  /// Maximum element volume in the primary lower-dimensional subdomain
  std::vector<Real> & _primary_lower_max_volume;
  /// Minimum element volume in the primary lower-dimensional subdomain
  std::vector<Real> & _primary_lower_min_volume;
  /// Median element volume in the primary lower-dimensional subdomain
  std::vector<Real> & _primary_lower_median_volume;

  /// Element count in the mortar segment mesh, one entry per mortar interface
  std::vector<unsigned int> & _msm_n_elems;
  /// Maximum element volume in the mortar segment mesh
  std::vector<Real> & _msm_max_volume;
  /// Minimum element volume in the mortar segment mesh
  std::vector<Real> & _msm_min_volume;
  /// Median element volume in the mortar segment mesh
  std::vector<Real> & _msm_median_volume;
};
