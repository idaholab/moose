//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMScalarBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMScalarBoundaryIntegratedBC);

InputParameters
MFEMScalarBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient", "The coefficient which will be used in the integrated BC");
  return params;
}

MFEMScalarBoundaryIntegratedBC::MFEMScalarBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarCoefficient(_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
