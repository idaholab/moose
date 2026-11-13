//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNonconservativeDGTraceIntegratedBC.h"

registerMooseObject("MooseApp", MFEMNonconservativeDGTraceIntegratedBC);

InputParameters
MFEMNonconservativeDGTraceIntegratedBC::validParams()
{
  InputParameters params = MFEMDGTraceIntegratedBC::validParams();
  params.addClassDescription(
      "Adds the boundary face integrator to an MFEM problem for the DG convection flux "
      "$-\\alpha \\langle \\rho_u (\\vec Q \\cdot \\hat n) \\{v\\}, [u] \\rangle + \\beta "
      "\\langle \\rho_u |\\vec Q \\cdot \\hat n| [v], [u] \\rangle$, with $u = v = 0$ outside "
      "the domain.");
  return params;
}

MFEMNonconservativeDGTraceIntegratedBC::MFEMNonconservativeDGTraceIntegratedBC(
    const InputParameters & parameters)
  : MFEMDGTraceIntegratedBC(parameters)
{
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMNonconservativeDGTraceIntegratedBC::createBFIntegrator()
{
  return new mfem::NonconservativeDGTraceIntegrator(_coef, _vec_coef, _alpha, _beta);
}

#endif
