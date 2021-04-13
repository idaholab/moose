//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorInterface.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "MooseTypes.h"
#include "MooseObject.h"
#include "UserObject.h"

InputParameters
PostprocessorInterface::validParams()
{
  return emptyInputParameters();
}

PostprocessorInterface::PostprocessorInterface(const MooseObject * moose_object)
  : _ppi_moose_object(*moose_object),
    _ppi_params(_ppi_moose_object.parameters()),
    _ppi_feproblem(*_ppi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & param_name,
                                              const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueHelper(param_name, index, 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOld(const std::string & param_name,
                                                 const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueHelper(param_name, index, 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlder(const std::string & param_name,
                                                   const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueHelper(param_name, index, 2);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOldByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlderByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 2);
}

bool
PostprocessorInterface::isDefaultPostprocessorValue(const std::string & param_name,
                                                    const unsigned int index /* = 0 */) const
{
  checkParam(param_name, index);

  return _ppi_params.isDefaultPostprocessorValue(param_name, index);
}

bool
PostprocessorInterface::hasPostprocessor(const std::string & param_name,
                                         const unsigned int index /* = 0 */) const
{
  if (!postprocessorsAdded())
    _ppi_moose_object.mooseError(
        "Cannot call hasPostprocessor() until all Postprocessors have been constructed.");

  // If the parameter is a default value, we don't actually create a Postprocessor object
  // for it, therefore this will be false
  if (isDefaultPostprocessorValue(param_name, index))
    return false;

  return hasPostprocessorByName(getPostprocessorName(param_name, index));
}

bool
PostprocessorInterface::hasPostprocessorByName(const PostprocessorName & name) const
{
  if (!postprocessorsAdded())
    _ppi_moose_object.mooseError(
        "Cannot call hasPostprocessorByName() until all Postprocessors have been constructed.");

  return _ppi_feproblem.getReporterData().hasReporterValue<PostprocessorValue>(
      PostprocessorReporterName(name));
}

std::size_t
PostprocessorInterface::coupledPostprocessors(const std::string & param_name) const
{
  checkParam(param_name);

  if (_ppi_params.isType<PostprocessorName>(param_name))
    return 1;
  return _ppi_params.get<std::vector<PostprocessorName>>(param_name).size();
}

void
PostprocessorInterface::checkParam(
    const std::string & param_name,
    const unsigned int index /* = std::numeric_limits<unsigned int>::max() */) const
{
  const bool check_index = index != std::numeric_limits<unsigned int>::max();

  if (!_ppi_params.isParamValid(param_name))
    _ppi_moose_object.mooseError(
        "When getting a Postprocessor, failed to get a parameter with the name \"",
        param_name,
        "\".",
        "\n\nKnown parameters:\n",
        _ppi_moose_object.parameters());

  if (_ppi_params.isType<PostprocessorName>(param_name))
  {
    if (check_index && index > 0)
      _ppi_moose_object.paramError(param_name,
                                   "A Postprocessor was requested with index ",
                                   index,
                                   " but a single Postprocessor is coupled.");
  }
  else if (_ppi_params.isType<std::vector<PostprocessorName>>(param_name))
  {
    const auto & names = _ppi_params.get<std::vector<PostprocessorName>>(param_name);
    if (check_index && names.size() <= index)
      _ppi_moose_object.paramError(param_name,
                                   "A Postprocessor was requested with index ",
                                   index,
                                   " but only ",
                                   names.size(),
                                   " Postprocessors are coupled.");
  }
  else
  {
    _ppi_moose_object.mooseError(
        "Supplied parameter with name \"",
        param_name,
        "\" of type \"",
        _ppi_params.type(param_name),
        "\" is not an expected type for getting a Postprocessor.\n\n",
        "Allowed types are \"PostprocessorName\" and \"std::vector<PostprocessorName>\".");
  }
}

const PostprocessorName &
PostprocessorInterface::getPostprocessorName(const std::string & param_name,
                                             const unsigned int index /* = 0 */) const
{
  // If the Postprocessor is a default value, its name is actually the default value,
  // therefore we shouldn't be getting a name associated with it
  if (isDefaultPostprocessorValue(param_name, index))
    _ppi_moose_object.mooseError(
        "While getting the name of the Postprocessor from parameter \"",
        param_name,
        "\" at index ",
        index,
        ":\nCannot get the name because the Postprocessor is a defualt value.");

  return _ppi_params.isType<PostprocessorName>(param_name)
             ? _ppi_params.get<PostprocessorName>(param_name)
             : _ppi_params.get<std::vector<PostprocessorName>>(param_name)[index];
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueHelper(const std::string & param_name,
                                                    unsigned int index,
                                                    std::size_t t_index) const
{
  // If the Postprocessor is a default value (either set by addParam or set to a constant
  // value by the user in input/an action), we covert the name to a value, store said
  // value locally, and return it so that it fits in with the rest of the interface
  // (needing a reference to a value)
  if (isDefaultPostprocessorValue(param_name, index))
  {
    const auto key = std::make_pair(param_name, index);
    const auto value = _ppi_params.getDefaultPostprocessorValue(param_name, index);
    const auto & value_ref =
        *_default_values.emplace(key, libmesh_make_unique<PostprocessorValue>(value))
             .first->second; // first is inserted pair, second is value in pair
    mooseAssert(value == value_ref, "Inconsistent default value");
    return value_ref;
  }
  // If not a default and all pps have been added, we check check for existance
  else if (postprocessorsAdded() && !hasPostprocessor(param_name, index))
    _ppi_moose_object.paramError(param_name,
                                 "A Postprocessor with the name \"",
                                 getPostprocessorName(param_name, index),
                                 "\" was not found.");

  return getPostprocessorValueByNameHelper(getPostprocessorName(param_name, index), t_index);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByNameHelper(const PostprocessorName & name,
                                                          std::size_t t_index) const
{
  mooseAssert(t_index < 3, "Invalid time index");

  // If all pps have been added, we can check for existance
  if (postprocessorsAdded() && !hasPostprocessorByName(name))
    _ppi_moose_object.mooseError("A Postprocessor with the name \"", name, "\" was not found.");

  if (t_index == 0)
    addPostprocessorDependencyHelper(name);

  return _ppi_feproblem.getReporterData().getReporterValue<PostprocessorValue>(
      PostprocessorReporterName(name), _ppi_moose_object, REPORTER_MODE_ROOT, t_index);
}

bool
PostprocessorInterface::postprocessorsAdded() const
{
  return _ppi_feproblem.getMooseApp().actionWarehouse().isTaskComplete("add_postprocessor");
}
