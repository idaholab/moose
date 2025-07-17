#ifdef MFEM_ENABLED
#pragma once


#include "MFEMGeneralUserObject.h"

/*
Class to construct threshold refiner.

The underlying mfem::ThresholdRefiner needs to be initialised with a
reference to the estimator.

Making all the methods dummies for now.
*/
class MFEMThresholdMarker : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMThresholdMarker(const InputParameters & params);

  virtual ~MFEMThresholdMarker() = default;

  void setUp();

  std::shared_ptr<mfem::ParFiniteElementSpace>
  getFESpace();

  // Mark the array. Return true if it's time to stop, either because the mesh is sufficiently
  // refined, or because we have done enough steps
  bool MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements);

  // Return true if it's time to stop, either because the mesh is sufficiently
  // refined, or because we have done enough steps
  bool Apply(mfem::ParMesh & mesh);

  //! Return whether these functions are enabled
  bool UseHRefinement() const { return _use_h_refinement; }
  bool UsePRefinement() const { return _use_p_refinement; }

  int MaxHLevel() const { return _max_h_level; }
  int MaxPLevel() const { return _max_p_level; }

protected:
  // Shared pointer to underlying mfem object
  std::shared_ptr<mfem::ThresholdRefiner> _threshold_refiner;
  std::string _estimator_name;
  
  float _error_threshold;
  int _steps; // total number of refinement steps to take
  int _max_h_level;
  int _max_p_level;
  std::string _refinement_type;

  bool _use_h_refinement{false};
  bool _use_p_refinement{false};
};

#endif
