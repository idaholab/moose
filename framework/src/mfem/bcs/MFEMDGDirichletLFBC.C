//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDirichletLFBC.h"

registerMooseObject("MooseApp", MFEMDGDirichletLFBC);

InputParameters
MFEMDGDirichletLFBC::validParams()
{
  InputParameters params = MFEMDGBoundaryCondition::validParams();
  params.addClassDescription("Boundary condition for dirichlet lf");
  return params;
}

MFEMDGDirichletLFBC::MFEMDGDirichletLFBC(const InputParameters & parameters)
  : MFEMDGBoundaryCondition(parameters)
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMDGDirichletLFBC::createLFIntegrator()
{
  return new mfem::DGDirichletLFIntegrator(_zero, _one, _sigma, _kappa);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMDGDirichletLFBC::createBFIntegrator()
{
  return nullptr;
}

#endif
