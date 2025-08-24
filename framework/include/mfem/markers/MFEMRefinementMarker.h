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

  void setUp();

  std::shared_ptr<mfem::ParFiniteElementSpace> getFESpace();

  /// Modifies the input array with all the locations in the mesh where we should increase
  /// the polynomial order
  void MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements);

  /// Refines the mesh wherever the refiner sees fit.
  void HRefine(mfem::ParMesh & mesh);

  /// Checks if H refinement is enabled, and if we should continue.
  bool UseHRefinement() const
  {
    return _use_h_refinement and !_stop_h_ref and (_h_ref_counter < _max_h_level);
  }

  /// Checks if P refinement is enabled, and if we should continue.
  bool UsePRefinement() const
  {
    return _use_p_refinement and !_stop_p_ref and (_p_ref_counter < _max_p_level);
  }

  const unsigned & maxHLevel() const { return _max_h_level; }
  const unsigned & maxPLevel() const { return _max_p_level; }

protected:
  // Shared pointer to underlying mfem object
  std::shared_ptr<mfem::ThresholdRefiner> _threshold_refiner;
  std::string _estimator_name;

  float _error_threshold;
  const unsigned _max_h_level;
  const unsigned _max_p_level;
  unsigned _h_ref_counter{0};
  unsigned _p_ref_counter{0};

  /// Bool to indicate we have reached stopping condition
  /// for h-refinement.
  bool _stop_h_ref{false};

  /// Bool to indicate we have reached stopping condition
  /// for p-refinement.
  bool _stop_p_ref{false};

  std::string _refinement_type;

  bool _use_h_refinement{false};
  bool _use_p_refinement{false};
};

#endif
