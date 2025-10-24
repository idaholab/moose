//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGradientGridFunction.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGradientGridFunction);

InputParameters
MFEMGradientGridFunction::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Function");
  params.addClassDescription("Creates a GradientGridFunctionCoefficient out of a variable");
  params.addRequiredParam<VariableName>("var_name", "The names of the gridfunction variable");
  return params;
}

MFEMGradientGridFunction::MFEMGradientGridFunction(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), _var_name(getParam<VariableName>("var_name"))
{
  // declares GradientGridFunctionCoefficient
  getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
      name(), getMFEMProblem().getProblemData().gridfunctions.Get(_var_name));
}

MFEMGradientGridFunction::~MFEMGradientGridFunction() {}

#endif
