//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEML1Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEML1Error);

InputParameters
MFEML1Error::validParams()
{
  InputParameters params = MFEMNormPostprocessor::validParams();
  params.addClassDescription(
      "Computes the L1 error $\\left\\Vert u_{ex} - u_{h}\\right\\Vert_{\\rm L1}$ for "
      "gridfunctions using H1 or L2 elements.");
  params.addRequiredParam<MFEMScalarCoefficientName>("function",
                                                     "The analytic solution to compare against.");
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the variable of which to find the norm of the error.");
  return params;
}

MFEML1Error::MFEML1Error(const InputParameters & parameters)
  : MFEMNormPostprocessor(parameters),
    _coeff(getScalarCoefficient("function")),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable")))
{
}

PostprocessorValue
MFEML1Error::getValue() const
{
  return _var.ComputeL1Error(_coeff, getIntegrationRules());
}

#endif
