//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorL2Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorL2Error);

InputParameters
MFEMVectorL2Error::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert \\vec u_{ex} - \\vec u_{h}\\right\\Vert_{\\rm L2}$ for "
      "vector gridfunctions.");
  params.addParam<MFEMVectorCoefficientName>("function",
                                             "The analytic solution to compare against.");
  params.addParam<VariableName>(
      "variable", "Name of the vector variable of which to find the norm of the error.");
  return params;
}

MFEMVectorL2Error::MFEMVectorL2Error(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _vec_coeff(getVectorCoefficient("function")),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(getParam<VariableName>("variable")))
{
}

void
MFEMVectorL2Error::initialize()
{
}

void
MFEMVectorL2Error::execute()
{
}

PostprocessorValue
MFEMVectorL2Error::getValue() const
{
  return _var.ComputeL2Error(_vec_coeff);
}

#endif
