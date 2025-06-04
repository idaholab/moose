#ifdef MFEM_ENABLED

#include "MFEMZienkiewiczZhuEstimator.h"

registerMooseObject("MooseApp", MFEMZienkiewiczZhuEstimator);

InputParameters
MFEMZienkiewiczZhuEstimator::validParams()
{
  return MFEMEstimator::validParams();
}

// Make sure we don't do this until all the grid functions etc are set up!
MFEMZienkiewiczZhuEstimator::MFEMZienkiewiczZhuEstimator(const InputParameters & params)
  : MFEMEstimator(params)
{}


mfem::ErrorEstimator * MFEMZienkiewiczZhuEstimator::createEstimator()
{
  MFEMProblemData & problem = getMFEMProblem().getProblemData();
  mfem::BilinearFormIntegrator* integ;
  
  // fetch the kernel first so we can get the blf integrator
  const UserObject * kernel_uo = &(getMFEMProblem().getUserObjectBase(_kernel_name));
  if (dynamic_cast<const MFEMKernel *>(kernel_uo) != nullptr)
  {
    std::shared_ptr<MooseObject> object_ptr = getMFEMProblem().getUserObject<MFEMKernel>(_kernel_name).getSharedPtr();
    std::shared_ptr<MFEMKernel> kernel      = std::dynamic_pointer_cast<MFEMKernel>(object_ptr);
    integ                                   = kernel->createBFIntegrator();
  }
  else
  {
    mooseError("Could not fetch kernel with name ", _kernel_name);
  }
  
  // fetch the order, dim and sdim
  auto fec  = problem.fecs.GetShared( _fe_space_name );
  int order = fec->GetOrder();
  
  int dim  = problem.pmesh->Dimension();
  int sdim = problem.pmesh->SpaceDimension();
  
  /*
  Set up error estimator. As per example 6p, we supply a space for the discontinuous
  flux (L2) and a space for the smoothed flux.
  */
  _flux_fec = std::make_unique<mfem::L2_FECollection>(order, dim);
  _flux_fes = std::make_unique<mfem::ParFiniteElementSpace>(problem.pmesh.get(), _flux_fec.get(), sdim);
  
  _smooth_flux_fec = std::make_unique<mfem::H1_FECollection>(order, dim);
  _smooth_flux_fes = std::make_unique<mfem::ParFiniteElementSpace>(problem.pmesh.get(), _smooth_flux_fec.get(), dim);
  
  // fetch the grid function we need
  auto gridfunction = problem.gridfunctions.GetShared( _test_var_name );
  
  // finally, initialise the estimator
  _error_estimator = std::make_shared<mfem::L2ZienkiewiczZhuEstimator>( *integ, *gridfunction, *_flux_fes, *_smooth_flux_fes );
  return _error_estimator.get();
}

#endif
