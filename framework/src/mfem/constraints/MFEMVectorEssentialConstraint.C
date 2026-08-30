//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorEssentialConstraint.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMVectorEssentialConstraint);

InputParameters
MFEMVectorEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params.addClassDescription(
      "Strongly constrains a vector variable in the specified subdomain(s).");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", "0. 0. 0.", "The vector coefficient setting the constrained values");
  return params;
}

MFEMVectorEssentialConstraint::MFEMVectorEssentialConstraint(const InputParameters & parameters)
  : MFEMEssentialConstraint(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

void
MFEMVectorEssentialConstraint::ApplyConstraint(mfem::ParGridFunction & gridfunc,
                                               mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & attrs = getSubdomainAttributes();
  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc, _vec_coef, attrs);
  Moose::MFEM::subdomainTrueDofs(*gridfunc.ParFESpace(), attrs, ess_tdof_list);
}

#endif
