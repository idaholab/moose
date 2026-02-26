//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorL2Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexVectorL2Error);

InputParameters
MFEMComplexVectorL2Error::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert \\vec u_{ex} - \\vec u_{h}\\right\\Vert_{\\rm L2}$ for "
      "complex vector gridfunctions.");
  params.addParam<MFEMVectorCoefficientName>("function_real",
                                             "The analytic real part of the solution to compare against.");
  params.addParam<MFEMVectorCoefficientName>("function_imag",
                                             "The analytic imaginary part of the solution to compare against.");
  params.addParam<VariableName>(
      "variable", "Name of the vector variable of which to find the norm of the error.");
  return params;
}

MFEMComplexVectorL2Error::MFEMComplexVectorL2Error(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _vec_coeff_real(getVectorCoefficient("function_real")),
    _vec_coeff_imag(getVectorCoefficient("function_imag")),
    _var(getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(getParam<VariableName>("variable")))
{
}

void
MFEMComplexVectorL2Error::initialize()
{
}

void
MFEMComplexVectorL2Error::execute()
{
}

PostprocessorValue
MFEMComplexVectorL2Error::getValue() const
{
  return _var.ComputeL2Error(_vec_coeff_real, _vec_coeff_imag);
}

#endif
