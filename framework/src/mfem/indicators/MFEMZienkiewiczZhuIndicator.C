//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMZienkiewiczZhuIndicator.h"
#include "MFEMProblem.h"
#include "MFEMKernel.h"

registerMooseObject("MooseApp", MFEMZienkiewiczZhuIndicator);

InputParameters
MFEMZienkiewiczZhuIndicator::validParams()
{
  return MFEMIndicator::validParams();
}

// Make sure we don't do this until all the grid functions etc are set up!
MFEMZienkiewiczZhuIndicator::MFEMZienkiewiczZhuIndicator(const InputParameters & params)
  : MFEMIndicator(params)
{
}

void
MFEMZienkiewiczZhuIndicator::createEstimator()
{
  MFEMProblemData & problem = getMFEMProblem().getProblemData();
  mfem::BilinearFormIntegrator * integ;

  // fetch the kernel first so we can get the blf integrator
  mooseAssert(dynamic_cast<const MFEMKernel *>(&(getMFEMProblem().getUserObjectBase(_kernel_name))),
              "Could not fetch kernel with name " + _kernel_name);
  std::shared_ptr<MooseObject> object_ptr =
      getMFEMProblem().getUserObject<MFEMKernel>(_kernel_name).getSharedPtr();
  std::shared_ptr<MFEMKernel> kernel = std::dynamic_pointer_cast<MFEMKernel>(object_ptr);
  integ = kernel->createBFIntegrator();

  // Next, we need to check that this integrator is supported by mfem::L2ZienkiewiczZhuEstimator
  [[maybe_unused]] bool is_supported = false;

  // Check it correctly casts into DiffusionIntegrator
  is_supported |= (dynamic_cast<mfem::DiffusionIntegrator *>(integ) != nullptr);
  // Check it correctly casts into CurlCurlIntegrator
  is_supported |= (dynamic_cast<mfem::CurlCurlIntegrator *>(integ) != nullptr);
  // Check it correctly casts into ElasticityIntegrator
  is_supported |= (dynamic_cast<mfem::ElasticityIntegrator *>(integ) != nullptr);

  mooseAssert(is_supported,
              "MFEMZienkiewiczZhuIndicator only supports MFEMDiffusionKernel, MFEMCurlCurlKernel "
              "and MFEMLinearElasticityKernel");

  int order = getFESpace().GetMaxElementOrder();

  int dim = problem.pmesh->Dimension();
  int sdim = problem.pmesh->SpaceDimension();

  // If we are using a Curl-curl integrator, we use a different space for the (smoothed) fluxes
  if (dynamic_cast<mfem::CurlCurlIntegrator *>(integ) != nullptr)
  {
    _flux_fec = std::make_unique<mfem::RT_FECollection>(order - 1, sdim);
    _flux_fes = std::make_unique<mfem::ParFiniteElementSpace>(problem.pmesh.get(), _flux_fec.get());

    _smooth_flux_fec = std::make_unique<mfem::ND_FECollection>(order, dim);
    _smooth_flux_fes =
        std::make_unique<mfem::ParFiniteElementSpace>(problem.pmesh.get(), _smooth_flux_fec.get());
  }

  /*
  Set up error estimator. As per example 6p, we supply a space for the discontinuous
  flux (L2) and a space for the smoothed flux. This branch should be the default option
  */
  else
  {
    _flux_fec = std::make_unique<mfem::L2_FECollection>(order, sdim);
    _flux_fes =
        std::make_unique<mfem::ParFiniteElementSpace>(problem.pmesh.get(), _flux_fec.get(), sdim);

    _smooth_flux_fec = std::make_unique<mfem::H1_FECollection>(order, dim);
    _smooth_flux_fes = std::make_unique<mfem::ParFiniteElementSpace>(
        problem.pmesh.get(), _smooth_flux_fec.get(), dim);
  }

  // fetch the grid function we need
  auto gridfunction = problem.gridfunctions.GetShared(_var_name);

  // finally, initialise the estimator
  _error_estimator = std::make_shared<mfem::L2ZienkiewiczZhuEstimator>(
      *integ, *gridfunction, *_flux_fes, *_smooth_flux_fes);
}

#endif
