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
#include "FEProblem.h"

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
      "indicates that data within the VPP is distributed and no auto communication is performed, "
      "this setting will result in parallel output within the CSV output; 'replicated' indicates "
      "that the data within the VPP is correct on processor 0, the data will automatically be "
      "broadcast to all processors unless the '_auto_broadcast' param is set to false within the "
      "validParams function.");

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const MooseObject * moose_object)
  : OutputInterface(moose_object->parameters()),
    _vpp_name(MooseUtils::shortName(moose_object->parameters().get<std::string>("_object_name"))),
    _vpp_fe_problem(
        *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _parallel_type(moose_object->parameters().get<MooseEnum>("parallel_type")),
    _vpp_moose_object(*moose_object),
    _vpp_tid(moose_object->parameters().isParamValid("_tid")
                 ? moose_object->parameters().get<THREAD_ID>("_tid")
                 : 0),
    _contains_complete_history(moose_object->parameters().get<bool>("contains_complete_history")),
    _is_distributed(_parallel_type == "DISTRIBUTED"),
    _is_broadcast(_is_distributed || !moose_object->parameters().get<bool>("_auto_broadcast"))
{
}

VectorPostprocessorValue &
VectorPostprocessor::declareVector(const std::string & vector_name)
{
  _vector_names.insert(vector_name);

  if (_vpp_tid)
    return _thread_local_vectors.emplace(vector_name, VectorPostprocessorValue()).first->second;

  // _is_broadcast = true (_auto_broadcast = false) then data is produced in a replicated manner
  ReporterMode mode = REPORTER_MODE_ROOT;
  if (_is_broadcast)
    mode = REPORTER_MODE_REPLICATED;
  if (_is_distributed)
    mode = REPORTER_MODE_DISTRIBUTED;

  return _vpp_fe_problem.getReporterData(ReporterData::WriteKey())
      .declareReporterValue<VectorPostprocessorValue,
                            VectorPostprocessorContext<VectorPostprocessorValue>>(
          VectorPostprocessorReporterName(_vpp_name, vector_name), mode, _vpp_moose_object);
}

const std::set<std::string> &
VectorPostprocessor::getVectorNames() const
{
  return _vector_names;
}

const ReporterMode REPORTER_MODE_VPP_SCATTER("VPP_SCATTER");

template <typename T>
VectorPostprocessorContext<T>::VectorPostprocessorContext(const libMesh::ParallelObject & other,
                                                          const MooseObject & producer,
                                                          ReporterState<T> & state)
  : ReporterGeneralContext<T>(other, producer, state)
{
}

template <typename T>
void
VectorPostprocessorContext<T>::finalize()
{
  ReporterGeneralContext<T>::finalize();

  const auto & consumer_modes = this->state().getConsumers();
  auto func = [](const std::pair<ReporterMode, const MooseObject *> & mode_pair)
  { return mode_pair.first == REPORTER_MODE_VPP_SCATTER; };
  if (std::find_if(consumer_modes.begin(), consumer_modes.end(), func) != consumer_modes.end())
  {
    const T & value = this->state().value();
    if (this->processor_id() == 0 && value.size() != this->n_processors())
      mooseError("The VectorPostprocessor value to be scatter has a length of ",
                 value.size(),
                 "; it must be the same length as the number of processors (",
                 this->n_processors(),
                 ").");

    this->comm().scatter(value, _scatter_value);
  }
}

template <typename T>
void
VectorPostprocessorContext<T>::copyValuesBack()
{
  ReporterGeneralContext<T>::copyValuesBack();
  _scatter_value_old = _scatter_value;
}

template <typename T>
const ScatterVectorPostprocessorValue &
VectorPostprocessorContext<T>::getScatterValue() const
{
  return _scatter_value;
}

template <typename T>
const ScatterVectorPostprocessorValue &
VectorPostprocessorContext<T>::getScatterValueOld() const
{
  return _scatter_value_old;
}

// Explicit instantiation
template class VectorPostprocessorContext<VectorPostprocessorValue>;
