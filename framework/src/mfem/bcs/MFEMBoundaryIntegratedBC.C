//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMBoundaryIntegratedBC);

InputParameters
MFEMBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "The coefficient which will be used in the integrated BC");
  return params;
}

MFEMBoundaryIntegratedBC::MFEMBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
<<<<<<< HEAD:framework/src/mfem/bcs/MFEMBoundaryIntegratedBC.C
mfem::LinearFormIntegrator *
MFEMBoundaryIntegratedBC::createLFIntegrator()
=======
std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMScalarBoundaryIntegratedBC::createLFIntegrator()
>>>>>>> 858d7ab200 (Add complex option to kernels and integrated BCs):framework/src/mfem/bcs/MFEMScalarBoundaryIntegratedBC.C
{
  std::cout << "FIX THE COEFFICIENT ISSUE WITH COMPLEX KERNELS" << std::endl;
  return std::make_pair(new mfem::BoundaryLFIntegrator(_coef), getParam<MooseEnum>("numeric_type") == "real" ? nullptr
                                                                                                  : new mfem::BoundaryLFIntegrator(_coef));
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
<<<<<<< HEAD:framework/src/mfem/bcs/MFEMBoundaryIntegratedBC.C
mfem::BilinearFormIntegrator *
MFEMBoundaryIntegratedBC::createBFIntegrator()
=======
std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMScalarBoundaryIntegratedBC::createBFIntegrator()
>>>>>>> 858d7ab200 (Add complex option to kernels and integrated BCs):framework/src/mfem/bcs/MFEMScalarBoundaryIntegratedBC.C
{
  return std::make_pair(nullptr, nullptr);
}

#endif
