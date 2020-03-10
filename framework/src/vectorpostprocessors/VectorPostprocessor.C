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

defineLegacyParams(VectorPostprocessor);

InputParameters
VectorPostprocessor::validParams()
{
  InputParameters params = UserObject::validParams();
  params += OutputInterface::validParams();
  params.addParam<bool>("contains_complete_history",
                        false,
                        "Set this flag to indicate that the values in all vectors declared by this "
                        "VPP represent a time history (e.g. with each invocation, new values are "
                        "added and old values are never removed). This changes the output so that "
                        "only a single file is output and updated with each invocation");

  // VPPs can set this to true if their resulting vectors are naturally replicated in parallel
  // setting this to false will keep MOOSE from unnecessarily broadcasting those vectors
  params.addPrivateParam<bool>("_auto_broadcast", true);

  // VPPs can operate in "distributed" mode, which disables the automatic broadcasting
  // and results in an individual file per processor if CSV output is enabled
  MooseEnum parallel_type("DISTRIBUTED REPLICATED", "REPLICATED");
  params.addParam<MooseEnum>(
      "parallel_type",
      parallel_type,
      "Set how the data is represented within the VectorPostprocessor (VPP); 'distributed' "
      "indicates that data within the VPP is distributed and no auto communication is preformed, "
      "this setting will result in parallel output within the CSV output; 'replicated' indicates "
      "that the data within the VPP is correct on processor 0, the data will automatically be "
      "broadcast to all processors unless the '_auto_broadcast' param is set to false within the "
      "validParams function.");

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters)
  : OutputInterface(parameters),
    _vpp_name(MooseUtils::shortName(parameters.get<std::string>("_object_name"))),
    _vpp_fe_problem(parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _parallel_type(parameters.get<MooseEnum>("parallel_type")),
    _vpp_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _contains_complete_history(parameters.get<bool>("contains_complete_history")),
    _is_distributed(_parallel_type == "DISTRIBUTED"),
    _is_broadcast(_is_distributed || !parameters.get<bool>("_auto_broadcast"))
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
        _vpp_name, vector_name, _contains_complete_history, _is_broadcast, _is_distributed);
}
