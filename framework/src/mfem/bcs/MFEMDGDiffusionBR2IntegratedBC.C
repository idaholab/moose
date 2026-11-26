//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGDiffusionBR2IntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGDiffusionBR2IntegratedBC);

InputParameters
MFEMDGDiffusionBR2IntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Adds the boundary face integrator to an MFEM problem for the BR2 DG diffusion "
      "stabilization term $\\sum_e \\eta (r_e([u]), r_e([v]))$, with $u = v = 0$ outside the "
      "domain.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for diffusion coefficient k.");
  params.addParam<mfem::real_t>(
      "eta", 1.0, "Penalty parameter. A value of one gives a stable discretization.");
  return params;
}

MFEMDGDiffusionBR2IntegratedBC::MFEMDGDiffusionBR2IntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _fespace(*getMFEMProblem().getGridFunction(_test_var_name)->ParFESpace()),
    _coef(getScalarCoefficient("coefficient")),
    _eta(getParam<mfem::real_t>("eta"))
{
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMDGDiffusionBR2IntegratedBC::createBFIntegrator()
{
  return new mfem::DGDiffusionBR2Integrator(_fespace, _coef, _eta);
}

#endif
