//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNonconservativeDGTraceKernel.h"

registerMooseObject("MooseApp", MFEMNonconservativeDGTraceKernel);

InputParameters
MFEMNonconservativeDGTraceKernel::validParams()
{
  InputParameters params = MFEMDGTraceKernel::validParams();
  params.addClassDescription(
      "Adds the interior face integrator to an MFEM problem for the DG convection flux "
      "$-\\alpha \\langle \\rho_u (\\vec Q \\cdot \\hat n) \\{v\\}, [u] \\rangle + \\beta "
      "\\langle \\rho_u |\\vec Q \\cdot \\hat n| [v], [u] \\rangle$ arising from the "
      "non-conservative DG discretization of the operator $\\vec Q \\cdot \\vec\\nabla u$.");
  return params;
}

MFEMNonconservativeDGTraceKernel::MFEMNonconservativeDGTraceKernel(
    const InputParameters & parameters)
  : MFEMDGTraceKernel(parameters)
{
}

mfem::BilinearFormIntegrator *
MFEMNonconservativeDGTraceKernel::createBFIntegrator()
{
  return new mfem::NonconservativeDGTraceIntegrator(_coef, _vec_coef, _alpha, _beta);
}

#endif
