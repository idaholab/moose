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
  params.addClassDescription("Projects a a real and imaginary vector coefficient onto a complex vector MFEM auxvariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient_real",
                                                     "Name of the real part of the vector coefficient to project.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient_imag",
                                                     "Name of the imaginary part of the vector coefficient to project.");
  return params;
}

MFEMComplexVectorProjectionAux::MFEMComplexVectorProjectionAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters), _vec_coef_real(getVectorCoefficient("vector_coefficient_real")), _vec_coef_imag(getVectorCoefficient("vector_coefficient_imag"))
{
}

void
MFEMComplexVectorProjectionAux::execute()
{
  _result_var.real().ProjectCoefficient(_vec_coef_real);
  _result_var.imag().ProjectCoefficient(_vec_coef_imag);
}

#endif
