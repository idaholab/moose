//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarDirichletBC.h"

registerMooseObject("MooseApp", MFEMScalarDirichletBC);

InputParameters
MFEMScalarDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the values on the essential boundary");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag",
      "The imaginary part of the coefficient setting the values on the essential boundary");

  return params;
}

MFEMScalarDirichletBC::MFEMScalarDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"))),
    _coef_imag(isParamValid("coefficient_imag")
                   ? getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient_imag"))
                   : getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

void
MFEMScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef, getBoundaryMarkers());
}

void
MFEMScalarDirichletBC::ApplyComplexBC(mfem::ParComplexGridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef, _coef_imag, getBoundaryMarkers());
}

#endif
