//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexScalarProjectionAux.h"

registerMooseObject("MooseApp", MFEMComplexScalarProjectionAux);

InputParameters
MFEMComplexScalarProjectionAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription("Projects a real and imaginary scalar coefficient onto a complex scalar MFEM auxvariable");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient_real",
                                                     "Name of the real part of the scalar coefficient to project.");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient_imag",
                                                     "Name of the imaginary part of the scalar coefficient to project.");
  return params;
}

MFEMComplexScalarProjectionAux::MFEMComplexScalarProjectionAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters), _coef_real(getScalarCoefficient("coefficient_real")), _coef_imag(getScalarCoefficient("coefficient_imag"))
{
}

void
MFEMComplexScalarProjectionAux::execute()
{
  _result_var.real().ProjectCoefficient(_coef_real);
  _result_var.imag().ProjectCoefficient(_coef_imag);
}

#endif
