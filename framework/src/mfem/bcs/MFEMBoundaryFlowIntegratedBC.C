//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryFlowIntegratedBC.h"

registerMooseObject("MooseApp", MFEMBoundaryFlowIntegratedBC);

InputParameters
MFEMBoundaryFlowIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Adds the boundary face integrator to an MFEM problem for the linear form "
      "$\\frac{\\alpha}{2} \\langle (\\vec Q \\cdot \\hat n) f, v \\rangle - \\beta \\langle "
      "|\\vec Q \\cdot \\hat n| f, v \\rangle$, which imposes the inflow value $f$ in DG "
      "discretizations of convection.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for the inflow value f.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the velocity coefficient Q to use.");
  params.addParam<mfem::real_t>(
      "alpha",
      -1.0,
      "Weight of the normal flux term. Use the negative of the alpha of the matching DG trace "
      "term on the left hand side.");
  params.addParam<mfem::real_t>(
      "beta",
      "Weight of the absolute normal flux term. Defaults to alpha/2. Use the negative of the "
      "beta of the matching DG trace term on the left hand side.");
  return params;
}

MFEMBoundaryFlowIntegratedBC::MFEMBoundaryFlowIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef(getScalarCoefficient("coefficient")),
    _vec_coef(getVectorCoefficient("vector_coefficient")),
    _alpha(getParam<mfem::real_t>("alpha")),
    _beta(isParamValid("beta") ? getParam<mfem::real_t>("beta") : 0.5 * _alpha)
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMBoundaryFlowIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryFlowIntegrator(_coef, _vec_coef, _alpha, _beta);
}

#endif
