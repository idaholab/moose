#ifdef MFEM_ENABLED
#pragma once


#include "MFEMGeneralUserObject.h"

/*
Class to construct threshold refiner.

The underlying mfem::ThresholdRefiner needs to be initialised with a
reference to the estimator.

Making all the methods dummies for now.
*/
class MFEMRefinementMarker : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMRefinementMarker(const InputParameters & params);

  virtual ~MFEMRefinementMarker() = default;

  void setUp();

  std::shared_ptr<mfem::ParFiniteElementSpace>
  getFESpace();

  void MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements);
  void HRefine(mfem::ParMesh & mesh);

  //! Return whether these functions are enabled
  //! Checking the the counter here is indeed redundant since the
  //! H-refinement and P-refinement functions modify this one as well
  bool UseHRefinement() const { return _use_h_refinement and !_stop_h_ref and (_h_ref_counter < _max_h_level); }
  bool UsePRefinement() const { return _use_p_refinement and !_stop_p_ref and (_p_ref_counter < _max_p_level); }

  int MaxHLevel() const { return _max_h_level; }
  int MaxPLevel() const { return _max_p_level; }

protected:
  // Shared pointer to underlying mfem object
  std::shared_ptr<mfem::ThresholdRefiner> _threshold_refiner;
  std::string _estimator_name;
  
  float _error_threshold;
  int _max_h_level;
  int _max_p_level;
  int _h_ref_counter{0};
  int _p_ref_counter{0};

  //! Bool to indicate we have reached stopping condition
  //! for h-refinement.
  bool _stop_h_ref{false};

  //! Bool to indicate we have reached stopping condition
  //! for p-refinement.
  bool _stop_p_ref{false};

  std::string _refinement_type;

  bool _use_h_refinement{false};
  bool _use_p_refinement{false};
};

#endif
