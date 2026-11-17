//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMH1Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMH1Error);

InputParameters
MFEMH1Error::validParams()
{
  InputParameters params = MFEMNormPostprocessor::validParams();
  params.addClassDescription(
      "Computes the H1 error $\\left\\Vert u_{ex} - u_{h}\\right\\Vert_{\\rm H1}$ for scalar "
      "gridfunctions using H1 elements.");
  params.addRequiredParam<MFEMScalarCoefficientName>("function",
                                                     "The analytic solution to compare against.");
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "gradient", "The gradient of the analytic solution, with one component per dimension.");
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the variable of which to find the norm of the error.");
  return params;
}

MFEMH1Error::MFEMH1Error(const InputParameters & parameters)
  : MFEMNormPostprocessor(parameters),
    _coeff(getScalarCoefficient("function")),
    _grad_coeff(getVectorCoefficient("gradient")),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable")))
{
  const int sdim = _var.FESpace()->GetMesh()->SpaceDimension();
  if (_grad_coeff.GetVDim() != sdim)
    paramError("gradient",
               "must have one component per spatial dimension of the mesh (",
               sdim,
               "), but has ",
               _grad_coeff.GetVDim(),
               ".");
}

PostprocessorValue
MFEMH1Error::getValue() const
{
  return _var.ComputeH1Error(&_coeff, &_grad_coeff, getIntegrationRules());
}

#endif
