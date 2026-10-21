//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorProjectionAux.h"

registerMooseObject("MooseApp", MFEMComplexVectorProjectionAux);

InputParameters
MFEMComplexVectorProjectionAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription(
      "Projects a real and imaginary vector coefficient, optionally scaled by a real scalar "
      "coefficient, onto a complex vector MFEM auxvariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient_real", "Name of the real part of the vector coefficient to project.");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient_imag",
      "Name of the imaginary part of the vector coefficient to project.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient",
      "1.",
      "Name of the real scalar coefficient k to scale both parts of the vector coefficient by.");
  return params;
}

MFEMComplexVectorProjectionAux::MFEMComplexVectorProjectionAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _vec_coef_real(getScalarCoefficient("coefficient"),
                   getVectorCoefficient("vector_coefficient_real")),
    _vec_coef_imag(getScalarCoefficient("coefficient"),
                   getVectorCoefficient("vector_coefficient_imag"))
{
}

void
MFEMComplexVectorProjectionAux::execute()
{
  _result_var.ProjectCoefficient(_vec_coef_real, _vec_coef_imag);
}

#endif
