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

registerMooseObject("MooseApp", MFEMRefinementMarker);

InputParameters
MFEMRefinementMarker::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Marker");

  MooseEnum refinement_type("h p hp", "hp");
  params.addRequiredParam<MooseEnum>(
      "refinement_type",
      refinement_type,
      "Specifies whether to use h-refinement, p-refinement or both.");

  params.addRequiredParam<Real>("refine", "Error fraction for the refiner");
  params.addRangeCheckedParam<Real>("refine",
                                    0,
                                    "refine>=0 & refine<=1",
                                    "Elements within this percentage of the max error will "
                                    "be refined.  Must be between 0 and 1!");
  params.addRangeCheckedParam<unsigned>(
      "max_h_level", 0, "max_h_level>=0 & max_h_level<=10", "Max number of h-refinement steps");
  params.addRangeCheckedParam<unsigned>(
      "max_p_level", 0, "max_p_level>=0 & max_p_level<=10", "Max number of p-refinement steps");

  params.addRequiredParam<std::string>("indicator", "Estimator to use");
  return params;
}

MFEMRefinementMarker::MFEMRefinementMarker(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _estimator_name(getParam<std::string>("indicator")),
    _error_threshold(getParam<Real>("refine")),
    _max_h_level(getParam<unsigned>("max_h_level")),
    _max_p_level(getParam<unsigned>("max_p_level")),
    _refinement_type(getParam<MooseEnum>("refinement_type")),
    _use_h_refinement(_refinement_type == "h" || _refinement_type == "hp"),
    _use_p_refinement(_refinement_type == "p" || _refinement_type == "hp")
{
}

void
MFEMRefinementMarker::setUp()
{
  // fetch const ref to the estimator
  _estimator = &getUserObjectByName<MFEMIndicator>(_estimator_name);

  // Check if p-refinement is supported by the fespace supplied with the variable
  auto & fespace = _estimator->getFESpace();

  if (_use_p_refinement and !fespace.PRefinementSupported())
  {
    mooseWarning("Specified p-refinement on an unsupported FESpace or geometry. Only H1 and L2 are "
                 "supported by mfem");
    _use_p_refinement = false;
  }

  _threshold_refiner = std::make_unique<mfem::ThresholdRefiner>(*(_estimator->getEstimator()));
  _threshold_refiner->SetTotalErrorFraction(_error_threshold);
}

void
MFEMRefinementMarker::pRefineMarker(mfem::Array<mfem::Refinement> & refinements)
{
  mfem::ParMesh & mesh = _estimator->getParMesh();

  // Hand over to the underlying mfem object to find all the
  // places we should increase the polynomial order
  _threshold_refiner->MarkWithoutRefining(mesh, refinements);

  // We are doing p-refinement. Increase the counter
  // and check if we have exceeded the max number of
  // p-refinement steps
  _stop_p_ref = (++_p_ref_counter >= _max_p_level);

  // The stopping condition is essentially that the refinements
  // array is empty, i.e. the refiner didn't find anywhere on the mesh
  // that needed its polynomial order increasing. We do essentially
  // an allreduce to check this on all the ranks. Do |= so that we
  // stop if either one of the conditions is met.
  _stop_p_ref |= (mesh.ReduceInt(refinements.Size()) == 0LL);
}

// We poll the refiner to ask if we need to stop h-refinement
void
MFEMRefinementMarker::hRefine()
{
  mfem::ParMesh & mesh = _estimator->getParMesh();

  // Perform h-refinement
  _threshold_refiner->Apply(mesh);

  // Increase the counter and check if we have exceeded
  // the max number of refinement steps
  _stop_h_ref = (++_h_ref_counter >= _max_h_level);

  // Ask the refiner if we need to stop H-refinement
  _stop_h_ref |= _threshold_refiner->Stop();
}

mfem::ParFiniteElementSpace &
MFEMRefinementMarker::getFESpace()
{
  mooseAssert(_estimator, "Indicator is null");
  return _estimator->getFESpace();
}

#endif
