//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexScalarDirichletBC.h"

registerMooseObject("MooseApp", MFEMComplexScalarDirichletBC);

InputParameters
MFEMComplexScalarDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_real",
      "0.",
      "The coefficient setting the real part of the values on the essential boundary");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient_imag",
      "0.",
      "The coefficient setting the imaginary part of the values on the essential boundary");

  return params;
}

MFEMComplexScalarDirichletBC::MFEMComplexScalarDirichletBC(const InputParameters & parameters)
  : MFEMComplexEssentialBC(parameters),
    _coef_real(getScalarCoefficient("coefficient_real")),
    _coef_imag(getScalarCoefficient("coefficient_imag"))
{
}

void
MFEMComplexScalarDirichletBC::ApplyBC(mfem::ParComplexGridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef_real, _coef_imag, getBoundaryMarkers());
}

#endif
