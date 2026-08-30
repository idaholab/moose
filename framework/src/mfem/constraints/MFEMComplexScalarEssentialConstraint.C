//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexScalarEssentialConstraint.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMComplexScalarEssentialConstraint);

InputParameters
MFEMComplexScalarEssentialConstraint::validParams()
{
  InputParameters params = MFEMComplexEssentialConstraint::validParams();
  params.addClassDescription("Strongly constrains the real and imaginary parts of a complex scalar "
                             "variable in the specified subdomain(s).");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_real", "0.", "The coefficient setting the real part of the constrained values");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag",
      "0.",
      "The coefficient setting the imaginary part of the constrained values");
  return params;
}

MFEMComplexScalarEssentialConstraint::MFEMComplexScalarEssentialConstraint(
    const InputParameters & parameters)
  : MFEMComplexEssentialConstraint(parameters),
    _coef_real(getScalarCoefficient("coefficient_real")),
    _coef_imag(getScalarCoefficient("coefficient_imag"))
{
}

void
MFEMComplexScalarEssentialConstraint::ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                                                      mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & attrs = getSubdomainAttributes();
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.real(), _coef_real, attrs);
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.imag(), _coef_imag, attrs);
  gridfunc.Sync();
  Moose::MFEM::subdomainTrueDofs(*gridfunc.ParFESpace(), attrs, ess_tdof_list);
}

#endif
