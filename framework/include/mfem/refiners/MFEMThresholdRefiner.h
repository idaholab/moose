#ifdef MFEM_ENABLED
#pragma once


#include "MFEMGeneralUserObject.h"
#include "MFEMEstimator.h"

/*
Class to construct threshold refiner.

The underlying mfem::ThresholdRefiner needs to be initialised with a
reference to the estimator.

Making all the methods dummies for now.
*/
class MFEMThresholdRefiner : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMThresholdRefiner(const InputParameters & params);

  virtual ~MFEMThresholdRefiner() = default;

  void setUp(std::shared_ptr<MFEMEstimator>);

  // Mark the array. Return true if it's time to stop, either because the mesh is sufficiently
  // refined, or because we have done enough steps
  bool MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements);

  // Return true if it's time to stop, either because the mesh is sufficiently
  // refined, or because we have done enough steps
  bool Apply(mfem::ParMesh & mesh);

  bool UseHRefinement() const { return _use_h_refinement and (_h_ref_counter < _max_h_level) and NotExceededTotalSteps(); }
  bool UsePRefinement() const { return _use_p_refinement and (_p_ref_counter < _max_p_level) and NotExceededTotalSteps(); }
  bool NotExceededTotalSteps() const { return (_h_ref_counter + _p_ref_counter) < _steps; }

protected:
  // Shared pointer to underlying mfem object
  std::shared_ptr<mfem::ThresholdRefiner> _threshold_refiner;
  
  float _error_threshold;
  int _steps; // total number of refinement steps to take
  int _max_h_level;
  int _max_p_level;
  std::string _refinement_type;

  int _h_ref_counter{0};
  int _p_ref_counter{0};

  bool _use_h_refinement{false};
  bool _use_p_refinement{false};
  
};

#endif
