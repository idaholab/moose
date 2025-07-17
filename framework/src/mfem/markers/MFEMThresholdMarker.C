#ifdef MFEM_ENABLED

#include "MFEMThresholdMarker.h"

registerMooseObject("MooseApp", MFEMThresholdMarker);

InputParameters
MFEMThresholdMarker::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Marker");

  MooseEnum refinement_type("H_REF P_REF H_P_REF", "H_P_REF");
  params.addRequiredParam<MooseEnum>("refinement_type", refinement_type, "Specifies whether to use h-refinement, p-refinement or both.");

  params.addRequiredParam<Real>("refine", "Error fraction for the refiner");
  params.addRangeCheckedParam<Real>("refine",
                                    0,
                                    "refine>=0 & refine<=1",
                                    "Elements within this percentage of the max error will "
                                    "be refined.  Must be between 0 and 1!");
  params.addRangeCheckedParam<int>("max_h_level", -1, "max_h_level>=0 & max_h_level <= 10", "Total number of h-refinement steps");
  params.addRangeCheckedParam<int>("max_p_level", -1, "max_p_level>=0 & max_p_level <= 10", "Total number of p-refinement steps");

  params.addRequiredParam<std::string>("indicator", "Estimator to use");
  return params;
}

MFEMThresholdMarker::MFEMThresholdMarker(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _estimator_name(getParam<std::string>("indicator")),
    _error_threshold(getParam<Real>("refine")),
    _max_h_level(getParam<int>("max_h_level")),
    _max_p_level(getParam<int>("max_p_level")),
    _refinement_type(getParam<MooseEnum>("refinement_type"))
{
  // _use_h_refinement and _use_p_refinement both default to false
  if ( _refinement_type == "H_P_REF" )
  {
    _use_h_refinement = true;
    _use_p_refinement = true;
  }
  else if ( _refinement_type == "H_REF" )
  {
    _use_h_refinement = true;
  }
  else if ( _refinement_type == "P_REF" )
  {
    _use_p_refinement = true;
  }
}


void
MFEMThresholdMarker::setUp()
{
  // fetch const ref to the estimator
  const auto& estimator = getUserObjectByName<MFEMIndicator>(_estimator_name);

  _threshold_refiner = std::make_shared<mfem::ThresholdRefiner>( *(estimator.getEstimator()) );
  _threshold_refiner->SetTotalErrorFraction(_error_threshold);
}

bool
MFEMThresholdMarker::MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements)
{
  // We are doing p-refinement. Increase the counter
  // and check if we have exceeded the max number of
  // p-refinement steps
  _threshold_refiner->MarkWithoutRefining(mesh, refinements);

  bool output = (mesh.ReduceInt(refinements.Size()) == 0LL);

  return output;
}

// Returns true when it's time to stop - the refiner itself
// will tell us if we've finished refinement
bool
MFEMThresholdMarker::Apply(mfem::ParMesh & mesh)
{
  bool output = _threshold_refiner->Apply(mesh);
  return output;
}

std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMThresholdMarker::getFESpace()
{
  const auto& estimator = getUserObjectByName<MFEMIndicator>(_estimator_name);

  return estimator.getFESpace();
}


#endif
