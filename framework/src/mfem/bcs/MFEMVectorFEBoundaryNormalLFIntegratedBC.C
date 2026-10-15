//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEBoundaryNormalLFIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFEBoundaryNormalLFIntegratedBC);

InputParameters
MFEMVectorFEBoundaryNormalLFIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Adds the boundary integrator to an MFEM problem for the linear form $(\\vec f \\cdot \\hat "
      "n, \\vec v \\cdot \\hat n)_{\\partial\\Omega}$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Name of the vector coefficient to use.");
  return params;
}

MFEMVectorFEBoundaryNormalLFIntegratedBC::MFEMVectorFEBoundaryNormalLFIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
mfem::LinearFormIntegrator *
MFEMVectorFEBoundaryNormalLFIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorFEBoundaryNormalLFIntegrator(_vec_coef);
}

#endif
