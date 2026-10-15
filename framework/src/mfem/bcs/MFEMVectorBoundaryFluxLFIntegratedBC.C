//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorBoundaryFluxLFIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorBoundaryFluxLFIntegratedBC);

InputParameters
MFEMVectorBoundaryFluxLFIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(f, \\vec v \\cdot \\hat n)_{\\partial\\Omega}$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMVectorBoundaryFluxLFIntegratedBC::MFEMVectorBoundaryFluxLFIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
mfem::LinearFormIntegrator *
MFEMVectorBoundaryFluxLFIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryFluxLFIntegrator(_coef);
}

#endif
