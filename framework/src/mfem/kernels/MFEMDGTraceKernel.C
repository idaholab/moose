//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGTraceKernel.h"

registerMooseObject("MooseApp", MFEMDGTraceKernel);

InputParameters
MFEMDGTraceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the interior face integrator to an MFEM problem for the DG convection flux "
      "$\\alpha \\langle \\rho_u (\\vec Q \\cdot \\hat n) \\{u\\}, [v] \\rangle + \\beta "
      "\\langle \\rho_u |\\vec Q \\cdot \\hat n| [u], [v] \\rangle$ arising from the "
      "conservative DG discretization of the operator $\\vec\\nabla \\cdot (\\vec Q u)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient",
      "1.",
      "Name of property rho to use, evaluated on each face in the element the velocity points "
      "into.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the velocity coefficient Q to use.");
  params.addParam<mfem::real_t>("alpha", 1.0, "Weight of the average flux term.");
  params.addParam<mfem::real_t>(
      "beta", "Weight of the jump penalty term. Defaults to alpha/2, which gives the upwind flux.");
  return params;
}

MFEMDGTraceKernel::MFEMDGTraceKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _vec_coef(getVectorCoefficient("vector_coefficient")),
    _alpha(getParam<mfem::real_t>("alpha")),
    _beta(isParamValid("beta") ? getParam<mfem::real_t>("beta") : 0.5 * _alpha)
{
}

mfem::BilinearFormIntegrator *
MFEMDGTraceKernel::createBFIntegrator()
{
  return new mfem::DGTraceIntegrator(_coef, _vec_coef, _alpha, _beta);
}

#endif
