#ifdef MFEM_ENABLED

#include "MFEMRefiner.h"

registerMooseObject("MooseApp", MFEMRefiner);

InputParameters
MFEMRefiner::validParams()
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
  params.addRequiredParam<int>("steps", "Total number of refinement steps");
  params.addRangeCheckedParam<int>("max_h_level", -1, "max_h_level>=0 & max_h_level <= 10", "Total number of h-refinement steps");
  params.addRangeCheckedParam<int>("max_p_level", -1, "max_p_level>=0 & max_p_level <= 10", "Total number of p-refinement steps");
  return params;
}

MFEMRefiner::MFEMRefiner(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _error_threshold(getParam<Real>("refine")),
    _steps(getParam<int>("steps")),
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
MFEMRefiner::setUp(std::shared_ptr<MFEMEstimator> estimator)
{
  _threshold_refiner = std::make_shared<mfem::ThresholdRefiner>( *(estimator->getEstimator()) );
}

bool
MFEMRefiner::MarkWithoutRefining(mfem::ParMesh & mesh, mfem::Array<mfem::Refinement> & refinements)
{
  // We are doing p-refinement. Increase the counter
  // and check if we have exceeded the max number of
  // p-refinement steps
  bool output = (++_p_ref_counter >= _max_p_level);
  _threshold_refiner->MarkWithoutRefining(mesh, refinements);

  output |= (mesh.ReduceInt(refinements.Size()) == 0LL);

  return output;
}

// Returns true when it's time to stop
bool
MFEMRefiner::Apply(mfem::ParMesh & mesh)
{
  // We are doing h-refinement. Increase the counter
  // and check if we have exceeded the max number of
  // h-refinement steps
  bool output = (++_h_ref_counter >= _max_h_level);

  output |= _threshold_refiner->Apply(mesh);
  return output;
}

#endif
