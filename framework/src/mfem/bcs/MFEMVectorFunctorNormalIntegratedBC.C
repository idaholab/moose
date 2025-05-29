//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorNormalIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorNormalIntegratedBC);

InputParameters
MFEMVectorFunctorNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f \\cdot \\hat n, v)_{\\partial\\Omega}$");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Vector coefficient whose normal component will be used in the integrated BC. A coefficient "
      "can be any of the following: a variable, an MFEM material property, a function, a "
      "post-processor, or a numeric value.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorNormalIntegratedBC::MFEMVectorFunctorNormalIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef_name(getParam<MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef(getVectorCoefficient(_vec_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctorNormalIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryNormalLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctorNormalIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
