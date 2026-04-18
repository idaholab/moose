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

registerMooseMFEMObject("MooseApp", ComplexScalarDirichletBC);

namespace Moose::MFEM
{
InputParameters
ComplexScalarDirichletBC::validParams()
{
  InputParameters params = ComplexEssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient_real",
      "0.",
      "The coefficient setting the real part of the values on the essential boundary");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient_imag",
      "0.",
      "The coefficient setting the imaginary part of the values on the essential boundary");

  return params;
}

ComplexScalarDirichletBC::ComplexScalarDirichletBC(const InputParameters & parameters)
  : ComplexEssentialBC(parameters),
    _coef_real(getScalarCoefficient("coefficient_real")),
    _coef_imag(getScalarCoefficient("coefficient_imag"))
{
}

void
ComplexScalarDirichletBC::ApplyBC(mfem::ParComplexGridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef_real, _coef_imag, getBoundaryMarkers());
}

} // namespace Moose::MFEM
#endif
