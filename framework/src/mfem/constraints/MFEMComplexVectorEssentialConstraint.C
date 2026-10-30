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
  mfem::ParFiniteElementSpace & pfes = *gridfunc.ParFESpace();
  auto check_fits = [&](const std::string & param, mfem::VectorCoefficient & coef)
  {
    if (!Moose::MFEM::coefficientFitsSpace(pfes, coef))
      paramError(param,
                 "The vector coefficient has ",
                 coef.GetVDim(),
                 " components, which does not fit the ",
                 pfes.GetVectorDim(),
                 "-component space of variable '",
                 _trial_var_name,
                 "'.");
  };
  check_fits("vector_coefficient_real", _vec_coef_real);
  check_fits("vector_coefficient_imag", _vec_coef_imag);

  // Bring the real and imaginary views up to date with the complex vector before
  // writing through them, and hand their state back afterwards.
  const mfem::Array<int> & markers = getSubdomainMarkers();
  gridfunc.Sync();
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.real(), _vec_coef_real, markers);
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc.imag(), _vec_coef_imag, markers);
  gridfunc.SyncAlias();
  ess_tdof_list.Append(getSubdomainTrueDofs(pfes));
}

#endif
