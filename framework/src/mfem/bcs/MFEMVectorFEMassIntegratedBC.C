//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEMassIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFEMassIntegratedBC);

InputParameters
MFEMVectorFEMassIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec u , \\vec v)_{\\partial\\Omega}$");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "The coefficient which will be used in the integrated BC.");
  return params;
}

MFEMVectorFEMassIntegratedBC::MFEMVectorFEMassIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
mfem::LinearFormIntegrator *
MFEMVectorFEMassIntegratedBC::createLFIntegrator()
{
  return nullptr;
}

// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFEMassIntegratedBC::createBFIntegrator()
{
  return new mfem::VectorFEMassIntegrator(_coef);;
}

#endif
