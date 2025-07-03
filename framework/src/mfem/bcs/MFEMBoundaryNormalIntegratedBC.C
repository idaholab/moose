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
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMBoundaryNormalIntegratedBC::MFEMBoundaryNormalIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
<<<<<<< HEAD:framework/src/mfem/bcs/MFEMBoundaryNormalIntegratedBC.C
mfem::LinearFormIntegrator *
MFEMBoundaryNormalIntegratedBC::createLFIntegrator()
=======
std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMVectorNormalIntegratedBC::createLFIntegrator()
>>>>>>> 858d7ab200 (Add complex option to kernels and integrated BCs):framework/src/mfem/bcs/MFEMVectorNormalIntegratedBC.C
{
  std::cout << "FIX THE COEFFICIENT ISSUE WITH COMPLEX KERNELS" << std::endl;
  return std::make_pair(new mfem::BoundaryNormalLFIntegrator(_vec_coef), getParam<MooseEnum>("numeric_type") == "real" ? nullptr
                                                                                                  : new mfem::BoundaryNormalLFIntegrator(_vec_coef));
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
<<<<<<< HEAD:framework/src/mfem/bcs/MFEMBoundaryNormalIntegratedBC.C
mfem::BilinearFormIntegrator *
MFEMBoundaryNormalIntegratedBC::createBFIntegrator()
=======
std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMVectorNormalIntegratedBC::createBFIntegrator()
>>>>>>> 858d7ab200 (Add complex option to kernels and integrated BCs):framework/src/mfem/bcs/MFEMVectorNormalIntegratedBC.C
{
  return std::make_pair(nullptr, nullptr);
}

#endif
