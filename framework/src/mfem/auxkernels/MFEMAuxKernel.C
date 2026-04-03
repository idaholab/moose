//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAuxKernel.h"
#include "MFEMProblem.h"

InputParameters
MFEMAuxKernel::validParams()
{
  InputParameters params = MFEMExecutedObject::validParams();
  params.registerBase("AuxKernel");
  params.registerSystemAttributeName("MFEMAuxKernel");
  params.addClassDescription("Base class for MFEM objects that update auxiliary variables outside "
                             "of the main solve step.");
  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");
  return params;
}

MFEMAuxKernel::MFEMAuxKernel(const InputParameters & parameters)
  : MFEMExecutedObject(parameters),
    _result_var_name(getParam<AuxVariableName>("variable")),
    _result_var(*getMFEMProblem().getGridFunction(_result_var_name))
{
}

std::set<std::string>
MFEMAuxKernel::consumedVariableNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<VariableName>(names, "source");
  appendTypedVectorParamIfValid<VariableName>(names, "source_variables");
  appendTypedParamIfValid<VariableName>(names, "first_source_vec");
  appendTypedParamIfValid<VariableName>(names, "second_source_vec");
  return names;
}

std::set<std::string>
MFEMAuxKernel::producedVariableNames() const
{
  return {_result_var_name};
}

#endif
