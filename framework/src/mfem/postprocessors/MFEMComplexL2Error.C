//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexL2Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexL2Error);

InputParameters
MFEMComplexL2Error::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert u_{ex} - u_{h}\\right\\Vert_{\\rm L2}$ for "
      "complex gridfunctions using H1 or L2 elements.");
  params.addParam<MFEMScalarCoefficientName>("function_real",
                                             "The analytic real part of the solution to compare against.");
  params.addParam<MFEMScalarCoefficientName>("function_imag",
                                             "The analytic imaginary part of the solution to compare against.");
  params.addParam<VariableName>("variable",
                                "Name of the variable of which to find the norm of the error.");
  return params;
}

MFEMComplexL2Error::MFEMComplexL2Error(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _coeff_real(getScalarCoefficient("function_real")),
    _coeff_imag(getScalarCoefficient("function_imag")),
    _var(getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(getParam<VariableName>("variable")))
{
}

void
MFEMComplexL2Error::initialize()
{
}

void
MFEMComplexL2Error::execute()
{
}

PostprocessorValue
MFEMComplexL2Error::getValue() const
{
  return _var.ComputeL2Error(_coeff_real, _coeff_imag);
}

#endif
