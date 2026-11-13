//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWhiteGaussianNoiseDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMWhiteGaussianNoiseDomainLFKernel);

InputParameters
MFEMWhiteGaussianNoiseDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the linear form "
      "$\\langle \\dot W, v \\rangle$, where $\\dot W$ is spatial white Gaussian noise.");
  params.addParam<unsigned int>(
      "seed",
      0,
      "Seed of the random number generator. A positive seed gives a reproducible sequence of "
      "samples; zero seeds the generator from the current time.");
  return params;
}

MFEMWhiteGaussianNoiseDomainLFKernel::MFEMWhiteGaussianNoiseDomainLFKernel(
    const InputParameters & parameters)
  : MFEMKernel(parameters),
    _comm(getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()->GetComm()),
    _seed(getParam<unsigned int>("seed"))
{
}

mfem::LinearFormIntegrator *
MFEMWhiteGaussianNoiseDomainLFKernel::createLFIntegrator()
{
  return new mfem::WhiteGaussianNoiseDomainLFIntegrator(_comm, _seed);
}

#endif
