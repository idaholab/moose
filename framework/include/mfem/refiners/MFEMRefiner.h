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
class MFEMRefiner : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMRefiner(const InputParameters & params);

  virtual ~MFEMRefiner() = default;

  void setUp(std::shared_ptr<MFEMEstimator>);

  // Mark the array. Return true if it's time to stop
  bool MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements);

  // return true if it's time to stop
  bool Apply(mfem::ParMesh & mesh);


protected:
  // Shared pointer to underlying mfem object
  std::shared_ptr<mfem::ThresholdRefiner> _threshold_refiner;

  float _error_threshold;
  int _steps; // total number of refinement steps to take
  int _max_h_level;
  int _max_p_level;

  int _h_ref_counter{0};
  int _p_ref_counter{0};
  
};

#endif
