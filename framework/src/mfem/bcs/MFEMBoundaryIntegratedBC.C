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
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag",
      "The imaginary part of the coefficient which will be used in the integrated BC. ");
  return params;
}

MFEMBoundaryIntegratedBC::MFEMBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    _coef_imag_name(isParamValid("coefficient_imag")
                        ? getParam<MFEMScalarCoefficientName>("coefficient_imag")
                        : getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef_imag(getScalarCoefficient(_coef_imag_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
MFEMBoundaryIntegratedBC::createLFIntegrator()
{
  return std::make_pair(new mfem::BoundaryLFIntegrator(_coef), isParamValid("coefficient_imag") ? new mfem::BoundaryLFIntegrator(_coef_imag)
                                                                                                  : nullptr);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMBoundaryIntegratedBC::createBFIntegrator()
{
  return std::make_pair(nullptr, nullptr);
}

#endif
