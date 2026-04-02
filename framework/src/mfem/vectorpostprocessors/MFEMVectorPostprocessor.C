//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorPostprocessor.h"

InputParameters
MFEMVectorPostprocessor::validParams()
{
  InputParameters params = MFEMExecutedObject::validParams();
  params += VectorPostprocessor::validParams();
  params.registerSystemAttributeName("MFEMVectorPostprocessor");
  return params;
}

MFEMVectorPostprocessor::MFEMVectorPostprocessor(const InputParameters & parameters)
  : MFEMExecutedObject(parameters), VectorPostprocessor(this)
{
}

std::set<std::string>
MFEMVectorPostprocessor::consumedVariableNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<VariableName>(names, "variable");
  appendTypedVectorParamIfValid<VariableName>(names, "variable");
  return names;
}

std::set<std::string>
MFEMVectorPostprocessor::producedVectorPostprocessorNames() const
{
  return {PPName()};
}

#endif // MOOSE_MFEM_ENABLED
