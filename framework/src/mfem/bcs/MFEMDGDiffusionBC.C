//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDiffusionBC.h"

registerMooseObject("MooseApp", MFEMDGDiffusionBC);

InputParameters
MFEMDGDiffusionBC::validParams()
{
  InputParameters params = MFEMDGBoundaryCondition::validParams();
  params.addClassDescription("Boundary condition for dirichlet lf");
  return params;
}

MFEMDGDiffusionBC::MFEMDGDiffusionBC(const InputParameters & parameters)
  : MFEMDGBoundaryCondition(parameters)
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMDGDiffusionBC::createLFIntegrator()
{
  return nullptr;
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMDGDiffusionBC::createBFIntegrator()
{
  return new mfem::DGDiffusionIntegrator(_one, _sigma, _kappa);
}

#endif
