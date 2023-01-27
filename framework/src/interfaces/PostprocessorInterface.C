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
#include "MooseObject.h"

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

PostprocessorInterface::PostprocessorInterface(const FEProblemBase * problem)
  : _ppi_moose_object(*problem),
    _ppi_params(_ppi_moose_object.parameters()),
    _ppi_feproblem(*problem)
{
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & param_name,
                                              const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueInternal(param_name, index, /* t_index = */ 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOld(const std::string & param_name,
                                                 const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueInternal(param_name, index, /* t_index = */ 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlder(const std::string & param_name,
                                                   const unsigned int index /* = 0 */) const
{
  return getPostprocessorValueInternal(param_name, index, /* t_index = */ 2);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameInternal(name, /* t_index = */ 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOldByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameInternal(name, /* t_index = */ 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlderByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameInternal(name, /* t_index = */ 2);
}

bool
PostprocessorInterface::isDefaultPostprocessorValue(const std::string & param_name,
                                                    const unsigned int index /* = 0 */) const
{
  return isDefaultPostprocessorValueByName(getPostprocessorNameInternal(param_name, index));
}

bool
PostprocessorInterface::isDefaultPostprocessorValueByName(const PostprocessorName & name) const
{
  // We do allow actual Postprocessor objects to have names that will succeed in being
  // represented as a double... so if the name does actually exist as a PP, it's not a default
  if (_ppi_feproblem.getReporterData().hasReporterValue<PostprocessorValue>(
          PostprocessorReporterName(name)))
    return false;

  std::istringstream ss(name);
  Real real_value = -std::numeric_limits<Real>::max();
  return (ss >> real_value && ss.eof());
}

PostprocessorValue
PostprocessorInterface::getDefaultPostprocessorValueByName(const PostprocessorName & name) const
{
  mooseAssert(isDefaultPostprocessorValueByName(name), "Not a default value");

  Real real_value = -std::numeric_limits<Real>::max();
  std::istringstream ss(name);
  ss >> real_value;
  return real_value;
}

bool
PostprocessorInterface::hasPostprocessor(const std::string & param_name,
                                         const unsigned int index /* = 0 */) const
{
  if (!postprocessorsAdded())
    _ppi_moose_object.mooseError(
        "Cannot call hasPostprocessor() until all Postprocessors have been constructed.");

  return hasPostprocessorByName(getPostprocessorNameInternal(param_name, index));
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
  return getPostprocessorNameInternal(param_name, index, /* allow_default_value = */ false);
}

const PostprocessorName &
PostprocessorInterface::getPostprocessorNameInternal(
    const std::string & param_name,
    const unsigned int index,
    const bool allow_default_value /* = true */) const
{
  checkParam(param_name, index);

  const auto & name = _ppi_params.isType<PostprocessorName>(param_name)
                          ? _ppi_params.get<PostprocessorName>(param_name)
                          : _ppi_params.get<std::vector<PostprocessorName>>(param_name)[index];

  if (!allow_default_value && isDefaultPostprocessorValueByName(name))
  {
    std::stringstream oss;
    oss << "Cannot get the name associated with PostprocessorName parameter \"" << param_name
        << "\"";
    if (index)
      oss << " at index " << index;
    oss << ",\nbecause said parameter is a default Postprocessor value.";
    _ppi_moose_object.paramError(param_name, oss.str());
  }

  return name;
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueInternal(const std::string & param_name,
                                                      unsigned int index,
                                                      std::size_t t_index) const
{
  const auto & name = getPostprocessorNameInternal(param_name, index);

  // If the Postprocessor is a default value (either set by addParam or set to a constant
  // value by the user in input/an action), we covert the name to a value, store said
  // value locally, and return it so that it fits in with the rest of the interface
  // (needing a reference to a value)
  if (isDefaultPostprocessorValueByName(name))
  {
    const auto value = getDefaultPostprocessorValueByName(name);
    const auto & value_ref =
        *_default_values.emplace(name, std::make_unique<PostprocessorValue>(value))
             .first->second; // first is inserted pair, second is value in pair
    mooseAssert(value == value_ref, "Inconsistent default value");
    return value_ref;
  }
  // If not a default and all pps have been added, we check check for existance
  else if (postprocessorsAdded() && !hasPostprocessorByName(name))
    _ppi_moose_object.paramError(
        param_name, "A Postprocessor with the name \"", name, "\" was not found.");

  return getPostprocessorValueByNameInternal(name, t_index);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByNameInternal(const PostprocessorName & name,
                                                            std::size_t t_index) const
{
  mooseAssert(t_index < 3, "Invalid time index");
  mooseAssert(!isDefaultPostprocessorValueByName(name), "Should not be default");

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
