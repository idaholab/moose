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

registerMooseObject("MooseApp", MFEMScalarEssentialConstraint);

InputParameters
MFEMScalarEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params.addClassDescription("Strongly constrains a scalar variable in the specified domain.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the values on the essential boundary");
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
  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  mfem::FiniteElementSpace * fes = gridfunc.FESpace();
  const mfem::Array<int> & domain_attrs = getSubdomainAttributes();
  for (int i = 0; i < fes->GetNE(); i++)
  {

    for (const auto attribute : domain_attrs)
    {
      if (fes->GetAttribute(i) != attribute)
        continue;

      fes->GetElementVDofs(i, vdofs, doftrans);
      vals.SetSize(vdofs.Size());
      fes->GetFE(i)->Project(_coef, *fes->GetElementTransformation(i), vals);
      doftrans.TransformPrimal(vals);
      gridfunc.SetSubVector(vdofs, vals);
    }
  }
}

#endif
