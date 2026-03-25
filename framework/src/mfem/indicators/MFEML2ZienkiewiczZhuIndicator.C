//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEML2ZienkiewiczZhuIndicator.h"
#include "MFEMProblem.h"
#include "MFEMKernel.h"

registerMooseObject("MooseApp", MFEML2ZienkiewiczZhuIndicator);

InputParameters
MFEML2ZienkiewiczZhuIndicator::validParams()
{
  InputParameters params = MFEMIndicator::validParams();
  params.addParam<UserObjectName>("flux_fespace", "FE space to write the flux into");
  params.addParam<UserObjectName>("smooth_flux_fespace", "FE space to write the smooth flux into");
  return params;
}

// Make sure we don't do this until all the grid functions etc are set up!
MFEML2ZienkiewiczZhuIndicator::MFEML2ZienkiewiczZhuIndicator(const InputParameters & params)
  : MFEMIndicator(params)
{
  if (isParamSetByUser("flux_fespace"))
  {
    // fetch the flux_fespace from the object system
    auto object_ptr = getUserObject<MFEMFESpace>("flux_fespace").getSharedPtr();
    auto fespace_ptr = std::dynamic_pointer_cast<const MFEMFESpace>(object_ptr);
    _flux_fes = fespace_ptr->getFESpace();
  }

  if (isParamSetByUser("smooth_flux_fespace"))
  {
    // fetch the smooth_flux_fespace from the object system
    auto object_ptr = getUserObject<MFEMFESpace>("smooth_flux_fespace").getSharedPtr();
    auto fespace_ptr = std::dynamic_pointer_cast<const MFEMFESpace>(object_ptr);
    _smooth_flux_fes = fespace_ptr->getFESpace();
  }
}

void
MFEML2ZienkiewiczZhuIndicator::createEstimator()
{
  // fetch the kernel first so we can build an auxiliary blf integrator
  MFEMKernel & kernel = getMFEMProblem().getUserObject<MFEMKernel>(_kernel_name);
  _integ = std::unique_ptr<mfem::BilinearFormIntegrator>(kernel.createBFIntegrator());

  // Next, we need to check that this integrator is supported by mfem::L2ZienkiewiczZhuEstimator
  [[maybe_unused]] bool is_supported = false;

  // Check it correctly casts into DiffusionIntegrator
  is_supported |= (dynamic_cast<mfem::DiffusionIntegrator *>(_integ.get()) != nullptr);
  // Check it correctly casts into CurlCurlIntegrator
  is_supported |= (dynamic_cast<mfem::CurlCurlIntegrator *>(_integ.get()) != nullptr);
  // Check it correctly casts into ElasticityIntegrator
  is_supported |= (dynamic_cast<mfem::ElasticityIntegrator *>(_integ.get()) != nullptr);

  mooseAssert(is_supported,
              "MFEML2ZienkiewiczZhuIndicator only supports MFEMDiffusionKernel, MFEMCurlCurlKernel "
              "and MFEMLinearElasticityKernel");

  int order = getFESpace().GetMaxElementOrder();
  int dim = getParMesh().Dimension();
  int sdim = getParMesh().SpaceDimension();

  if (!_flux_fes)
  {
    // not supplied by the user - default to L2
    _flux_fec = std::make_unique<mfem::L2_FECollection>(order, dim);
    _flux_fes = std::make_unique<mfem::ParFiniteElementSpace>(&getParMesh(), _flux_fec.get(), sdim);
  }

  if (!_smooth_flux_fes)
  {
    // not supplied by the user - default to H1
    _smooth_flux_fec = std::make_unique<mfem::H1_FECollection>(order, dim);
    _smooth_flux_fes =
        std::make_unique<mfem::ParFiniteElementSpace>(&getParMesh(), _smooth_flux_fec.get(), sdim);
  }

  // fetch the grid function we need
  auto gridfunction = getMFEMProblem().getProblemData().gridfunctions.GetShared(_var_name);

  // finally, initialise the estimator
  _error_estimator = std::make_shared<mfem::L2ZienkiewiczZhuEstimator>(
      *_integ, *gridfunction, *_flux_fes, *_smooth_flux_fes);
}

#endif
