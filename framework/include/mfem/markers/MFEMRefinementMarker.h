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

#include "MFEMGeneralUserObject.h"
#include "MFEMIndicator.h"

/**
 * Class to construct threshold refiner.
 * The underlying mfem::ThresholdRefiner needs to be initialised with a reference to the estimator.
 */
class MFEMRefinementMarker : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMRefinementMarker(const InputParameters & params);

  virtual ~MFEMRefinementMarker() = default;

  /// Constructs associated mfem::ThresholdRefiner once mfem::ErrorEstimator is guaranteed to exist
  void initialSetup();

  /// Applies p-refinement wherever the refiner sees fit
  bool pRefine();

  /// Applies h-refinement wherever the refiner sees fit
  bool hRefine();

protected:
  /// Unique pointer to underlying mfem::ThresholdRefiner object
  std::unique_ptr<mfem::ThresholdRefiner> _threshold_refiner;

  /// The estimator/indicator's name
  const std::string & _estimator_name;

  /// The error threshold determining which elements to refine
  const mfem::real_t _error_threshold;

  /// Whether to rebalance the mesh after h-refinement
  const bool _rebalance;

  /// The max no. of times h-refinement can be performed
  const unsigned _max_h_level;

  /// The max no. of times h-refinement can be performed
  const unsigned _max_p_level;

  /// The no. of times h-refinement has been performed
  unsigned _h_ref_counter{0};

  /// The no. of times p-refinement has been performed
  unsigned _p_ref_counter{0};

  /// Pointer to the estimator/indicator
  const MFEMIndicator * _estimator{nullptr};
};

#endif
