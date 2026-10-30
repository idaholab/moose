//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarEssentialConstraint.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMScalarEssentialConstraint);

InputParameters
MFEMScalarEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params.addClassDescription(
      "Strongly constrains a scalar variable in the specified subdomain(s).");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the constrained values");
  return params;
}

MFEMScalarEssentialConstraint::MFEMScalarEssentialConstraint(const InputParameters & parameters)
  : MFEMEssentialConstraint(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
MFEMScalarEssentialConstraint::ApplyConstraint(mfem::ParGridFunction & gridfunc,
                                               mfem::Array<int> & ess_tdof_list)
{
  if (gridfunc.ParFESpace()->GetVectorDim() != 1)
    paramError("variable", "not scalar; use MFEMVectorEssentialConstraint instead.");

  Moose::MFEM::projectCoefficientOnSubdomains(gridfunc, _coef, getSubdomainMarkers());
  ess_tdof_list.Append(getSubdomainTrueDofs(*gridfunc.ParFESpace()));
}

#endif
