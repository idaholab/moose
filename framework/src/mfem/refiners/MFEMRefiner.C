#ifdef MFEM_ENABLED

#include "MFEMRefiner.h"

registerMooseObject("MooseApp", MFEMRefiner);

InputParameters
MFEMRefiner::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Marker");

  params.addRequiredParam<Real>("refine", "Error fraction for the refiner");
  params.addRequiredParam<int>("steps", "Total number of refinement steps");
  params.addRequiredParam<int>("max_h_level", "Total number of h-refinement steps");
  params.addRequiredParam<int>("max_p_level", "Total number of p-refinement steps");
  return params;
}

MFEMRefiner::MFEMRefiner(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _error_threshold(getParam<Real>("refine")),
    _steps(getParam<int>("steps")),
    _max_h_level(getParam<int>("max_h_level")),
    _max_p_level(getParam<int>("max_p_level"))
{}


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
