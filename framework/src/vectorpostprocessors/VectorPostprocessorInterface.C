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

InputParameters
VectorPostprocessorInterface::validParams()
{
  return emptyInputParameters();
}

VectorPostprocessorInterface::VectorPostprocessorInterface(const MooseObject * moose_object,
                                                           bool broadcast_by_default)
  : _broadcast_by_default(broadcast_by_default),
    _vpi_moose_object(*moose_object),
    _vpi_feproblem(*_vpi_moose_object.parameters().getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base")),
    _vpi_tid(_vpi_moose_object.parameters().have_parameter<THREAD_ID>("_tid")
                 ? _vpi_moose_object.parameters().get<THREAD_ID>("_tid")
                 : 0)
{
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValue(const std::string & param_name,
                                                          const std::string & vector_name) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getVectorPostprocessorValueByName(getVectorPostprocessorName(param_name), vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  return getVectorPostprocessorByNameHelper(name, vector_name, _broadcast_by_default, 0);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOld(const std::string & param_name,
                                                             const std::string & vector_name) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getVectorPostprocessorValueOldByName(getVectorPostprocessorName(param_name), vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  return getVectorPostprocessorByNameHelper(name, vector_name, _broadcast_by_default, 1);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValue(const std::string & param_name,
                                                          const std::string & vector_name,
                                                          bool needs_broadcast) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getVectorPostprocessorValueByName(
      getVectorPostprocessorName(param_name), vector_name, needs_broadcast);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name,
    const std::string & vector_name,
    bool needs_broadcast) const
{
  return getVectorPostprocessorByNameHelper(
      name, vector_name, needs_broadcast || _broadcast_by_default, 0);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOld(const std::string & param_name,
                                                             const std::string & vector_name,
                                                             bool needs_broadcast) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getVectorPostprocessorValueOldByName(
      getVectorPostprocessorName(param_name), vector_name, needs_broadcast);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name,
    const std::string & vector_name,
    bool needs_broadcast) const
{
  return getVectorPostprocessorByNameHelper(
      name, vector_name, needs_broadcast || _broadcast_by_default, 1);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValue(
    const std::string & param_name, const std::string & vector_name) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getScatterVectorPostprocessorValueByName(getVectorPostprocessorName(param_name),
                                                  vector_name);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  return getVectorPostprocessorContextByNameHelper(name, vector_name).getScatterValue();
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueOld(
    const std::string & param_name, const std::string & vector_name) const
{
  possiblyCheckHasVectorPostprocessor(param_name, vector_name);
  return getScatterVectorPostprocessorValueOldByName(getVectorPostprocessorName(param_name),
                                                     vector_name);
}

const ScatterVectorPostprocessorValue &
VectorPostprocessorInterface::getScatterVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  return getVectorPostprocessorContextByNameHelper(name, vector_name).getScatterValueOld();
}

bool
VectorPostprocessorInterface::hasVectorPostprocessor(const std::string & param_name,
                                                     const std::string & vector_name) const
{
  if (!vectorPostprocessorsAdded())
    _vpi_feproblem.mooseError("Cannot call hasVectorPostprocessor() until all VectorPostprocessors "
                              "have been constructed.");

  return hasVectorPostprocessorByName(getVectorPostprocessorName(param_name), vector_name);
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorByName(const VectorPostprocessorName & name,
                                                           const std::string & vector_name) const
{
  if (!vectorPostprocessorsAdded())
    _vpi_feproblem.mooseError("Cannot call hasVectorPostprocessorByName() until all "
                              "VectorPostprocessors have been constructed.");

  const bool has_vpp = _vpi_feproblem.getReporterData().hasReporterValue<VectorPostprocessorValue>(
      VectorPostprocessorReporterName(name, vector_name));

  if (has_vpp)
    mooseAssert(_vpi_feproblem.hasUserObject(name) && dynamic_cast<const VectorPostprocessor *>(
                                                          &_vpi_feproblem.getUserObjectBase(name)),
                "Has reporter VectorPostprocessor Reporter value but not VectorPostprocessor UO");

  return has_vpp;
}

bool
VectorPostprocessorInterface::hasVectorPostprocessor(const std::string & param_name) const
{
  if (!vectorPostprocessorsAdded())
    _vpi_feproblem.mooseError("Cannot call hasVectorPostprocessor() until all "
                              "VectorPostprocessors have been constructed.");

  return hasVectorPostprocessorByName(getVectorPostprocessorName(param_name));
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorByName(
    const VectorPostprocessorName & name) const
{
  if (!vectorPostprocessorsAdded())
    _vpi_feproblem.mooseError("Cannot call hasVectorPostprocessorByName() until all "
                              "VectorPostprocessors have been constructed.");

  return _vpi_feproblem.hasUserObject(name) &&
         dynamic_cast<const VectorPostprocessor *>(&_vpi_feproblem.getUserObjectBase(name));
}

bool
VectorPostprocessorInterface::isVectorPostprocessorDistributed(const std::string & param_name) const
{
  return isVectorPostprocessorDistributedByName(getVectorPostprocessorName(param_name));
}

bool
VectorPostprocessorInterface::isVectorPostprocessorDistributedByName(
    const VectorPostprocessorName & name) const
{
  return _vpi_feproblem.getVectorPostprocessorObjectByName(name).isDistributed();
}

const VectorPostprocessorName &
VectorPostprocessorInterface::getVectorPostprocessorName(const std::string & param_name) const
{
  const auto & params = _vpi_moose_object.parameters();

  if (!params.isParamValid(param_name))
    _vpi_moose_object.mooseError(
        "When getting a VectorPostprocessor, failed to get a parameter with the name \"",
        param_name,
        "\".",
        "\n\nKnown parameters:\n",
        _vpi_moose_object.parameters());

  if (!params.isType<VectorPostprocessorName>(param_name))
    _vpi_moose_object.mooseError(
        "Supplied parameter with name \"",
        param_name,
        "\" of type \"",
        params.type(param_name),
        "\" is not an expected type for getting a VectorPostprocessor.\n\n",
        "The allowed type is \"VectorPostprocessorName\".");

  return params.get<VectorPostprocessorName>(param_name);
}

void
VectorPostprocessorInterface::possiblyCheckHasVectorPostprocessor(
    const std::string & param_name, const std::string & vector_name) const
{
  // Can't do checking if vpps have not been added
  if (!vectorPostprocessorsAdded())
    return;

  if (!hasVectorPostprocessor(param_name))
    _vpi_moose_object.paramError(param_name,
                                 "A VectorPostprocessor with the name \"",
                                 getVectorPostprocessorName(param_name),
                                 "\" was not found.");
  if (!hasVectorPostprocessor(param_name, vector_name))
    _vpi_moose_object.paramError(param_name,
                                 "The VectorPostprocessor \"",
                                 getVectorPostprocessorName(param_name),
                                 "\" does not have a vector named \"",
                                 vector_name,
                                 "\".");
}

void
VectorPostprocessorInterface::possiblyCheckHasVectorPostprocessorByName(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  // Can't do checking if vpps have not been added
  if (!vectorPostprocessorsAdded())
    return;

  if (!hasVectorPostprocessorByName(name))
    _vpi_moose_object.mooseError(
        "A VectorPostprocessor with the name \"", name, "\" was not found.");
  if (!hasVectorPostprocessorByName(name, vector_name))
    _vpi_moose_object.mooseError("The VectorPostprocessor \"",
                                 name,
                                 "\" does not have a vector named \"",
                                 vector_name,
                                 "\".");
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorByNameHelper(
    const VectorPostprocessorName & name,
    const std::string & vector_name,
    bool broadcast,
    std::size_t t_index) const
{
  possiblyCheckHasVectorPostprocessorByName(name, vector_name);
  addVectorPostprocessorDependencyHelper(name);

  const ReporterMode mode = broadcast ? REPORTER_MODE_REPLICATED : REPORTER_MODE_ROOT;
  return _vpi_feproblem.getReporterData().getReporterValue<VectorPostprocessorValue>(
      VectorPostprocessorReporterName(name, vector_name), _vpi_moose_object, mode, t_index);
}

const VectorPostprocessorContext<VectorPostprocessorValue> &
VectorPostprocessorInterface::getVectorPostprocessorContextByNameHelper(
    const VectorPostprocessorName & name, const std::string & vector_name) const
{
  possiblyCheckHasVectorPostprocessorByName(name, vector_name);
  addVectorPostprocessorDependencyHelper(name);

  // The complete name of the store Reporter value
  const VectorPostprocessorReporterName r_name(name, vector_name);

  // Indicate the scatter value is desired, so the the VectorPostprocessorContext will do scatter
  _vpi_feproblem.getReporterData().getReporterValue<VectorPostprocessorValue>(
      r_name, _vpi_moose_object, REPORTER_MODE_VPP_SCATTER, 0);

  // Retrieve the VectorPostprocessorContext which contains the scattered value to be referenced
  const auto & context = _vpi_feproblem.getReporterData().getReporterContextBase(r_name);
  auto vpp_context_ptr =
      dynamic_cast<const VectorPostprocessorContext<VectorPostprocessorValue> *>(&context);
  mooseAssert(vpp_context_ptr, "Failed to get the VectorPostprocessorContext");
  return *vpp_context_ptr;
}

bool
VectorPostprocessorInterface::vectorPostprocessorsAdded() const
{
  return _vpi_feproblem.getMooseApp().actionWarehouse().isTaskComplete("add_vector_postprocessor");
}
