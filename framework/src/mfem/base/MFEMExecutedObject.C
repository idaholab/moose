//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMExecutedObject.h"

InputParameters
MFEMExecutedObject::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;
  params.addClassDescription("Base class for executed MFEM objects.");
  return params;
}

MFEMExecutedObject::MFEMExecutedObject(const InputParameters & parameters)
  : MFEMObject(parameters), SetupInterface(this)
{
}

std::set<std::string>
MFEMExecutedObject::consumedVariableNames() const
{
  return {};
}

std::set<std::string>
MFEMExecutedObject::producedVariableNames() const
{
  return {};
}

std::set<std::string>
MFEMExecutedObject::consumedPostprocessorNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<PostprocessorName>(names, "postprocessor");
  appendTypedVectorParamIfValid<PostprocessorName>(names, "postprocessors");
  return names;
}

std::set<std::string>
MFEMExecutedObject::producedPostprocessorNames() const
{
  return {};
}

std::set<std::string>
MFEMExecutedObject::consumedVectorPostprocessorNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<VectorPostprocessorName>(names, "vectorpostprocessor");
  appendTypedParamIfValid<VectorPostprocessorName>(names, "vpp");
  appendTypedVectorParamIfValid<VectorPostprocessorName>(names, "vectorpostprocessors");
  return names;
}

std::set<std::string>
MFEMExecutedObject::producedVectorPostprocessorNames() const
{
  return {};
}

template <typename T>
void
MFEMExecutedObject::appendTypedParamIfValid(std::set<std::string> & names,
                                            const std::string & param_name) const
{
  if (parameters().isParamValid(param_name) && parameters().isType<T>(param_name))
    names.insert(parameters().get<T>(param_name));
}

template <typename T>
void
MFEMExecutedObject::appendTypedVectorParamIfValid(std::set<std::string> & names,
                                                  const std::string & param_name) const
{
  if (parameters().isParamValid(param_name) && parameters().isType<std::vector<T>>(param_name))
    for (const auto & name : parameters().get<std::vector<T>>(param_name))
      names.insert(name);
}

template void MFEMExecutedObject::appendTypedParamIfValid<PostprocessorName>(
    std::set<std::string> &, const std::string &) const;
template void MFEMExecutedObject::appendTypedParamIfValid<VectorPostprocessorName>(
    std::set<std::string> &, const std::string &) const;
template void MFEMExecutedObject::appendTypedParamIfValid<VariableName>(
    std::set<std::string> &, const std::string &) const;
template void MFEMExecutedObject::appendTypedVectorParamIfValid<PostprocessorName>(
    std::set<std::string> &, const std::string &) const;
template void MFEMExecutedObject::appendTypedVectorParamIfValid<VectorPostprocessorName>(
    std::set<std::string> &, const std::string &) const;
template void MFEMExecutedObject::appendTypedVectorParamIfValid<VariableName>(
    std::set<std::string> &, const std::string &) const;

#endif
