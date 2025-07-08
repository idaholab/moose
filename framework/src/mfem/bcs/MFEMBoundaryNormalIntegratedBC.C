//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryNormalIntegratedBC.h"

registerMooseObject("MooseApp", MFEMBoundaryNormalIntegratedBC);

InputParameters
MFEMBoundaryNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f \\cdot \\hat n, v)_{\\partial\\Omega}$");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "1. 1. 1.",
      "Vector coefficient whose normal component will be used in the integrated BC");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient_imag",
      "The imaginary part of the vector coefficient whose normal component will be used in the integrated BC. ");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMBoundaryNormalIntegratedBC::MFEMBoundaryNormalIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_coefficient"))),
    _vec_coef_imag_name(isParamValid("vector_coefficient_imag")
                            ? getParam<MFEMVectorCoefficientName>("vector_coefficient_imag")
                            : getParam<MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef_imag(getVectorCoefficient(_vec_coef_imag_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMBoundaryNormalIntegratedBC::createLFIntegrator()
{
  return std::make_pair(
      new mfem::BoundaryNormalLFIntegrator(_vec_coef),
      isParamValid("vector_coefficient_imag") ? new mfem::BoundaryNormalLFIntegrator(_vec_coef_imag)
                                               : nullptr);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMBoundaryNormalIntegratedBC::createBFIntegrator()
{
  return std::make_pair(nullptr, nullptr);
}

#endif
