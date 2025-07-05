//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorBoundaryIntegratedBC);

InputParameters
MFEMVectorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_{\\partial\\Omega}$");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Vector coefficient used in the boundary integrator");
  return params;
}

MFEMVectorBoundaryIntegratedBC::MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
