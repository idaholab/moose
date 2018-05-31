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
  params.addParam<bool>("contains_complete_history",
                        false,
                        "Set this flag to indicate that the values in all vectors declared by this "
                        "VPP represent a time history (e.g. with each invocation, new values are "
                        "added and old values are never removed). This changes the output so that "
                        "only a single file is output and updated with each invocation");

  // VPPs can set this to true if their resulting vectors are naturally replicated in parallel
  // setting this to true will keep MOOSE from unnecesarily broadcasting those vectors
  params.addPrivateParam<bool>("_is_broadcast", false);

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters)
  : OutputInterface(parameters),
    _vpp_name(MooseUtils::shortName(parameters.get<std::string>("_object_name"))),
    _vpp_fe_problem(parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vpp_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _contains_complete_history(parameters.get<bool>("contains_complete_history")),
    _is_broadcast(parameters.get<bool>("_is_broadcast"))
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
    return _vpp_fe_problem->declareVectorPostprocessorVector(
        _vpp_name, vector_name, _contains_complete_history, _is_broadcast);
}
