//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPostprocessor.h"

InputParameters
MFEMPostprocessor::validParams()
{
  InputParameters params = MFEMExecutedObject::validParams();
  params.registerSystemAttributeName("MFEMPostprocessor");
  params += Postprocessor::validParams();
  return params;
}

MFEMPostprocessor::MFEMPostprocessor(const InputParameters & parameters)
  : MFEMExecutedObject(parameters), Postprocessor(this)
{
}

std::set<std::string>
MFEMPostprocessor::consumedVariableNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<VariableName>(names, "variable");
  appendTypedParamIfValid<VariableName>(names, "primal_variable");
  appendTypedParamIfValid<VariableName>(names, "dual_variable");
  appendTypedVectorParamIfValid<VariableName>(names, "variable");
  return names;
}

std::set<std::string>
MFEMPostprocessor::producedPostprocessorNames() const
{
  return {name()};
}

#endif
