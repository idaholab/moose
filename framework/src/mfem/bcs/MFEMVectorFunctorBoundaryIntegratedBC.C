//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorBoundaryIntegratedBC);

InputParameters
MFEMVectorFunctorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_{\\partial\\Omega}$");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Vector coefficient used in the boundary integrator. A coefficient can be any of the "
      "following: a "
      "variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

MFEMVectorFunctorBoundaryIntegratedBC::MFEMVectorFunctorBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_coefficient")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
