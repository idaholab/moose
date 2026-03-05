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
 * The underlying mfem::ThresholdRefiner needs to be initialised with a
 * reference to the estimator.
 */
class MFEMRefinementMarker : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMRefinementMarker(const InputParameters & params);

  virtual ~MFEMRefinementMarker() = default;

  void initialSetup();

  /// Applies p-refinement wherever the refiner sees fit.
  bool pRefine();

  /// Applies h-refinement wherever the refiner sees fit.
  bool hRefine();

protected:
  /// Unique pointer to underlying mfem object
  std::unique_ptr<mfem::ThresholdRefiner> _threshold_refiner;
  const std::string & _estimator_name;
  const mfem::real_t _error_threshold;
  const bool _rebalance;
  const unsigned _max_h_level;
  const unsigned _max_p_level;
  unsigned _h_ref_counter{0};
  unsigned _p_ref_counter{0};

  const MFEMIndicator * _estimator{nullptr};
};

#endif
