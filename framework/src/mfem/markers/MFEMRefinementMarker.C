//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMRefinementMarker.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMRefinementMarker);

InputParameters
MFEMRefinementMarker::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Marker");

  params.addRequiredParam<std::string>("indicator", "Estimator to use");
  params.addRangeCheckedParam<Real>("threshold",
                                    0,
                                    "threshold>=0 & threshold<=1",
                                    "Elements above this percentage of the max error will "
                                    "be refined. Must be between 0 and 1!");
  params.addParam<bool>("rebalance", false, "Whether to rebalance the mesh after h-refinement");
  params.addRangeCheckedParam<unsigned>(
      "max_h_level", 0, "max_h_level>=0 & max_h_level<=10", "Max number of h-refinement steps");
  params.addRangeCheckedParam<unsigned>(
      "max_p_level", 0, "max_p_level>=0 & max_p_level<=10", "Max number of p-refinement steps");
  return params;
}

MFEMRefinementMarker::MFEMRefinementMarker(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _estimator_name(getParam<std::string>("indicator")),
    _error_threshold(getParam<Real>("threshold")),
    _rebalance(getParam<bool>("rebalance")),
    _max_h_level(getParam<unsigned>("max_h_level")),
    _max_p_level(getParam<unsigned>("max_p_level"))
{
}

void
MFEMRefinementMarker::initialSetup()
{
  // fetch const ref to the estimator
  _estimator = &getUserObjectByName<MFEMIndicator>(_estimator_name);

  // Check if p-refinement is supported by the fespace supplied with the variable
  if (_max_p_level and !_estimator->getFESpace().PRefinementSupported())
  {
    mooseWarning("Specified p-refinement on an unsupported FESpace or geometry. Only H1 and L2 "
                 "spaces on quad/hex meshes are supported by mfem. Disabling p-refinement.");
    _p_ref_counter = _max_p_level;
  }

  // For now, we lock out p-refinement, since we are unsure of the implementation.
  if (_max_p_level > 0)
  {
    mooseWarning("p-refinement is not supported at present.");
    _p_ref_counter = _max_p_level;
  }

  // We do not want to allow rebalancing if p-refinement is enabled, because that mesh-only
  // operation has no notion of p-refinement and the computational imbalance that may result.
  // Though p-refinement is not supported at this time, we add this in anyway for future-proofing.
  if (_max_p_level > 0 and _rebalance)
  {
    mooseWarning("Asked for rebalancing as well as p-refinement, which is not supported.");
    _rebalance = false;
  }

  _threshold_refiner = std::make_unique<mfem::ThresholdRefiner>(*(_estimator->getEstimator()));
  _threshold_refiner->SetTotalErrorFraction(_error_threshold);
}

bool
MFEMRefinementMarker::pRefine()
{
  // Nothing to do if we've reached the max level of refinement
  if (_p_ref_counter >= _max_p_level)
    return false;

  mfem::Array<mfem::Refinement> refinements;
  _threshold_refiner->MarkWithoutRefining(_estimator->getParMesh(), refinements);

  const bool refined = _estimator->getParMesh().ReduceInt(refinements.Size()) != 0LL;

  if (refined)
  {
    mfem::Array<mfem::pRefinement> prefinements(refinements.Size());
    for (const auto i : make_range(refinements.Size()))
      prefinements[i] = mfem::pRefinement(refinements[i].index, 1);

    // Perform p-refinement
    _estimator->getFESpace().PRefineAndUpdate(prefinements);
    // Update all gridfunctions since the same fespace can be shared by multiple gridfunctions
    getMFEMProblem().updateGridFunctions();
  }

  // Return whether we actually refined and increase the counter if we did
  return refined && ++_p_ref_counter;
}

bool
MFEMRefinementMarker::hRefine()
{
  // Nothing to do if we've reached the max level of refinement
  if (_h_ref_counter >= _max_h_level)
    return false;

  // Perform h-refinement
  _threshold_refiner->Apply(_estimator->getParMesh());

  const bool refined = !_threshold_refiner->Stop();

  if (refined)
  {
    // Update all fespaces since the same mesh can be shared by multiple fespaces
    getMFEMProblem().updateFESpaces();
    // Update all gridfunctions now we have updated all fespaces
    getMFEMProblem().updateGridFunctions();
    if (_rebalance)
      getMFEMProblem().rebalanceMesh(_estimator->getParMesh());
  }

  // Return whether we actually refined and increase the counter if we did
  return refined && ++_h_ref_counter;
}

#endif
