//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGKernel.h"

/*
TODO: these classes need their own headers and source files
*/

registerMooseObject("MooseApp", MFEMDGKernel);
registerMooseObject("MooseApp", MFEMDGDiffusionKernel);
registerMooseObject("MooseApp", MFEMDGDirichletLFKernel);

MFEMDGKernel::MFEMDGKernel(const InputParameters & parameters)
  : MFEMKernel(parameters) {}


MFEMDGDiffusionKernel::MFEMDGDiffusionKernel(const InputParameters & parameters)
  : MFEMDGKernel(parameters), _one(1.0), _zero(0.0)
{
}

mfem::BilinearFormIntegrator *
MFEMDGDiffusionKernel::createDGBFIntegrator()
{
  // One of the DG penalty parameters. Typically +/- 1.0
  mfem::real_t sigma(-1.0);
  // One of the DG penalty parameters. Negative values replaced with (order+1)^2
  mfem::real_t kappa(-1.0);

  return new mfem::DGDiffusionIntegrator(_one, sigma, kappa);
}


MFEMDGDirichletLFKernel::MFEMDGDirichletLFKernel(const InputParameters & parameters)
  : MFEMDGKernel(parameters), _one(1.0), _zero(0.0)
{
}

mfem::LinearFormIntegrator *
MFEMDGDirichletLFKernel::createDGLFIntegrator()
{
  // One of the DG penalty parameters. Typically +/- 1.0
  mfem::real_t sigma(-1.0);
  // One of the DG penalty parameters. Negative values replaced with (order+1)^2
  mfem::real_t kappa(-1.0);

  return new mfem::DGDirichletLFIntegrator(_zero, _one, sigma, kappa);
}


#endif
