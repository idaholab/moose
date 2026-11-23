//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHCurlError.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHCurlError);

InputParameters
MFEMHCurlError::validParams()
{
  InputParameters params = MFEMNormPostprocessor::validParams();
  params.addClassDescription(
      "Computes the H(curl) error $\\left\\Vert \\vec u_{ex} - \\vec u_{h}\\right\\Vert_{\\rm "
      "H(curl)}$ for vector gridfunctions using H(curl) elements.");
  params.addRequiredParam<MFEMVectorCoefficientName>("function",
                                                     "The analytic solution to compare against.");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "curl",
      "The curl of the analytic solution, with three components in 3D and one in 2D, where the "
      "curl is a scalar.");
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the vector variable of which to find the norm of the error.");
  return params;
}

MFEMHCurlError::MFEMHCurlError(const InputParameters & parameters)
  : MFEMNormPostprocessor(parameters),
    _vec_coeff(getVectorCoefficient("function")),
    _curl_coeff(getVectorCoefficient("curl")),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable")))
{
  if (_curl_coeff.GetVDim() != _var.CurlDim())
    paramError("curl",
               "must have as many components as the curl of the variable (",
               _var.CurlDim(),
               "), but has ",
               _curl_coeff.GetVDim(),
               ".");
}

PostprocessorValue
MFEMHCurlError::getValue() const
{
  return _var.ComputeHCurlError(&_vec_coeff, &_curl_coeff, getIntegrationRules());
}

#endif
