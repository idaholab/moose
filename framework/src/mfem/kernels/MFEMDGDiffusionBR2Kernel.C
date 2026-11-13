//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDiffusionBR2Kernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGDiffusionBR2Kernel);

InputParameters
MFEMDGDiffusionBR2Kernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the interior face integrator to an MFEM problem for the BR2 DG diffusion "
      "stabilization term $\\sum_e \\eta (r_e([u]), r_e([v]))$, where $r_e$ is the lifting "
      "operator of face $e$ weighted by the diffusion coefficient.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for diffusion coefficient k.");
  params.addParam<mfem::real_t>(
      "eta", 1.0, "Penalty parameter. A value of one gives a stable discretization.");
  return params;
}

MFEMDGDiffusionBR2Kernel::MFEMDGDiffusionBR2Kernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _fespace(*getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()),
    _coef(getScalarCoefficient("coefficient")),
    _eta(getParam<mfem::real_t>("eta"))
{
}

mfem::BilinearFormIntegrator *
MFEMDGDiffusionBR2Kernel::createBFIntegrator()
{
  return new mfem::DGDiffusionBR2Integrator(_fespace, _coef, _eta);
}

#endif
