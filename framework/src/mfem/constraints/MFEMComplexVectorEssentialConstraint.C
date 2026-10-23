//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorEssentialConstraint.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMComplexVectorEssentialConstraint);

InputParameters
MFEMComplexVectorEssentialConstraint::validParams()
{
  InputParameters params = MFEMComplexEssentialConstraint::validParams();
  params.addClassDescription("Strongly constrains the real and imaginary parts of a complex vector "
                             "variable in the specified subdomain(s).");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient_real",
      "0. 0. 0.",
      "The vector coefficient setting the real part of the constrained values");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient_imag",
      "0. 0. 0.",
      "The vector coefficient setting the imaginary part of the constrained values");
  return params;
}

MFEMComplexVectorEssentialConstraint::MFEMComplexVectorEssentialConstraint(
    const InputParameters & parameters)
  : MFEMComplexEssentialConstraint(parameters),
    _vec_coef_real(getVectorCoefficient("vector_coefficient_real")),
    _vec_coef_imag(getVectorCoefficient("vector_coefficient_imag"))
{
}

void
MFEMComplexVectorEssentialConstraint::ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                                                      mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & attrs = getSubdomainAttributes();
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.real(), _vec_coef_real, attrs);
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.imag(), _vec_coef_imag, attrs);
  gridfunc.Sync();
  Moose::MFEM::subdomainTrueDofs(*gridfunc.ParFESpace(), attrs, ess_tdof_list);
}

#endif
