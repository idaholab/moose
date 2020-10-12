//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorInterface.h"
#include "FEProblemBase.h"
#include "ReporterData.h"
#include "VectorPostprocessor.h"
#include "MooseTypes.h"
#include "UserObject.h"

VectorPostprocessorInterface::VectorPostprocessorInterface(const MooseObject * moose_object,
                                                           bool broadcast_by_default)
  : _broadcast_by_default(broadcast_by_default),
    _vpi_params(moose_object->parameters()),
    _vpi_feproblem(*_vpi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vpi_tid(_vpi_params.have_parameter<THREAD_ID>("_tid") ? _vpi_params.get<THREAD_ID>("_tid")
                                                           : 0),
    _vpi_reporter_data(_vpi_feproblem.getReporterDataInternal())
{
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValue(const std::string & name,
                                                          const std::string & vector_name)
{
  return getVectorPostprocessorValueByName(_vpi_params.get<VectorPostprocessorName>(name),
                                           vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name)
{
  return getVectorPostprocessorByNameHelper(name, vector_name, _broadcast_by_default, 0);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOld(const std::string & name,
                                                             const std::string & vector_name)
{
  return getVectorPostprocessorValueOldByName(_vpi_params.get<VectorPostprocessorName>(name),
                                              vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name, const std::string & vector_name)
{
  return getVectorPostprocessorByNameHelper(name, vector_name, _broadcast_by_default, 1);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValue(const std::string & name,
                                                          const std::string & vector_name,
                                                          bool needs_broadcast)
{
  return getVectorPostprocessorValueByName(
      _vpi_params.get<VectorPostprocessorName>(name), vector_name, needs_broadcast);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast)
{
  return getVectorPostprocessorByNameHelper(
      name, vector_name, needs_broadcast || _broadcast_by_default, 0);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOld(const std::string & name,
                                                             const std::string & vector_name,
                                                             bool needs_broadcast)
{
  return getVectorPostprocessorValueOldByName(
      _vpi_params.get<VectorPostprocessorName>(name), vector_name, needs_broadcast);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast)
{
  return getVectorPostprocessorByNameHelper(
      name, vector_name, needs_broadcast || _broadcast_by_default, 1);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValue(const std::string & name,
                                                                 const std::string & vector_name)
{
  return getScatterVectorPostprocessorValueByName(_vpi_params.get<VectorPostprocessorName>(name),
                                                  vector_name);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(
    const std::string & name, const std::string & vector_name)
{
  auto vpp_context_ptr = getVectorPostprocessorContextByNameHelper(name, vector_name);
  return vpp_context_ptr->getScatterValue();
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueOld(const std::string & name,
                                                                    const std::string & vector_name)
{
  return getScatterVectorPostprocessorValueOldByName(_vpi_params.get<VectorPostprocessorName>(name),
                                                     vector_name);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueOldByName(
    const std::string & name, const std::string & vector_name)
{
  auto vpp_context_ptr = getVectorPostprocessorContextByNameHelper(name, vector_name);
  return vpp_context_ptr->getScatterValueOld();
}

bool
VectorPostprocessorInterface::hasVectorPostprocessor(const std::string & name,
                                                     const std::string & vector_name) const
{
  return hasVectorPostprocessorByName(_vpi_params.get<VectorPostprocessorName>(name), vector_name);
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorByName(const VectorPostprocessorName & name,
                                                           const std::string & vector_name) const
{
  ReporterName r_name(name, vector_name);
  return _vpi_feproblem.getReporterData().hasReporterValue<VectorPostprocessorValue>(r_name);
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorObject(const std::string & name) const
{
  return hasVectorPostprocessorObjectByName(_vpi_params.get<VectorPostprocessorName>(name));
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorObjectByName(
    const VectorPostprocessorName & name) const
{
  if (_vpi_feproblem.hasUserObject(name))
  {
    const UserObject & uo = _vpi_feproblem.getUserObjectBase(name);
    return dynamic_cast<const VectorPostprocessor *>(&uo) != nullptr;
  }
  return false;
}

bool
VectorPostprocessorInterface::isVectorPostprocessorDistributed(const std::string & name) const
{
  return isVectorPostprocessorDistributedByName(_vpi_params.get<VectorPostprocessorName>(name));
}

bool
VectorPostprocessorInterface::isVectorPostprocessorDistributedByName(
    const VectorPostprocessorName & name) const
{
  const VectorPostprocessor & vpp_obj = _vpi_feproblem.getVectorPostprocessorObjectByName(name);
  return vpp_obj.isDistributed();
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorByNameHelper(const std::string & object_name,
                                                                 const std::string & vector_name,
                                                                 bool broadcast,
                                                                 std::size_t t_index) const
{
  ReporterMode mode = broadcast ? REPORTER_MODE_REPLICATED : REPORTER_MODE_ROOT;
  ReporterName r_name(object_name, vector_name);
  return _vpi_reporter_data.getReporterValue<VectorPostprocessorValue>(
      r_name, object_name, mode, t_index);
}

const VectorPostprocessorContext<VectorPostprocessorValue> *
VectorPostprocessorInterface::getVectorPostprocessorContextByNameHelper(
    const std::string & object_name, const std::string & vector_name) const
{
  // The complete name of the store Reporter value
  const ReporterName r_name(object_name, vector_name);

  // Indicate the scatter value is desired, so the the VectorPostprocessorContext will do scatter
  _vpi_reporter_data.getReporterValue<VectorPostprocessorValue>(
      r_name, object_name, REPORTER_MODE_VPP_SCATTER, 0);

  // Retrieve the VectorPostprocessorContext which contains the scattered value to be referenced
  const ReporterContextBase * context_ptr =
      _vpi_feproblem.getReporterData().getReporterContextBaseHelper(r_name);
  auto vpp_context_ptr =
      dynamic_cast<const VectorPostprocessorContext<VectorPostprocessorValue> *>(context_ptr);
  mooseAssert(vpp_context_ptr != nullptr, "Failed to get the VectorPostprocessorContext");
  return vpp_context_ptr;
}
