//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "VectorPostprocessor.h"
#include "SubProblem.h"
#include "Conversion.h"
#include "UserObject.h"
#include "VectorPostprocessorData.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<VectorPostprocessor>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<OutputInterface>();

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters)
  : OutputInterface(parameters),
    _vpp_name(MooseUtils::shortName(parameters.get<std::string>("_object_name"))),
    _vpp_fe_problem(parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vpp_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0)
{
}

VectorPostprocessorValue &
VectorPostprocessor::getVector(const std::string & vector_name)
{
  return _vpp_fe_problem->getVectorPostprocessorValue(_vpp_name, vector_name);
}

VectorPostprocessorValue &
VectorPostprocessor::declareVector(const std::string & vector_name)
{
  if (_vpp_tid)
    return _thread_local_vectors.emplace(vector_name, VectorPostprocessorValue()).first->second;
  else
    return _vpp_fe_problem->declareVectorPostprocessorVector(_vpp_name, vector_name);
}
