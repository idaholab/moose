//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputParameters.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"

#include "libmesh/utility.h"
#include "libmesh/simple_range.h"

#include "pcrecpp.h"

#include <cmath>

InputParameters
emptyInputParameters()
{
  InputParameters params;
  return params;
}

InputParameters::InputParameters()
  : Parameters(),
    _collapse_nesting(false),
    _moose_object_syntax_visibility(true),
    _show_deprecated_message(true),
    _allow_copy(true)
{
}

InputParameters::InputParameters(const InputParameters & rhs)
  : Parameters(), _show_deprecated_message(true), _allow_copy(true)
{
  *this = rhs;
}

InputParameters::InputParameters(const Parameters & rhs)
  : _show_deprecated_message(true), _allow_copy(true)
{
  _params.clear();
  Parameters::operator=(rhs);
  _collapse_nesting = false;
  _moose_object_syntax_visibility = true;
}

void
InputParameters::clear()
{
  Parameters::clear();
  _params.clear();
  _coupled_vars.clear();
  _new_to_deprecated_coupled_vars.clear();
  _collapse_nesting = false;
  _moose_object_syntax_visibility = true;
  _show_deprecated_message = true;
  _allow_copy = true;
  _block_fullpath = "";
  _block_location = "";
  _old_to_new_name_and_dep.clear();
  _new_to_old_names.clear();
}

void
InputParameters::addClassDescription(const std::string & doc_string)
{
  _class_description = doc_string;
}

void
InputParameters::set_attributes(const std::string & name_in, bool inserted_only)
{
  const auto name = checkForRename(name_in);

  if (!inserted_only)
  {
    /**
     * "._set_by_add_param" and ".deprecated_params" are not populated until after
     * the default value has already been set in libMesh (first callback to this
     * method). Therefore if a variable is in/not in one of these sets, you can
     * be assured it was put there outside of the "addParam*()" calls.
     */
    _params[name]._set_by_add_param = false;

    // valid_params don't make sense for MooseEnums
    if (!have_parameter<MooseEnum>(name) && !have_parameter<MultiMooseEnum>(name))
      _params[name]._valid = true;

    if (_show_deprecated_message)
    {
      auto emit_deprecation_message =
          [this](const auto & deprecated_name, const auto & deprecation_message)
      {
        const auto current_show_trace = Moose::show_trace;
        Moose::show_trace = false;
        moose::internal::mooseDeprecatedStream(Moose::out,
                                               false,
                                               false,
                                               errorPrefix(deprecated_name),
                                               ":\n",
                                               deprecation_message,
                                               "\n");
        Moose::show_trace = current_show_trace;
      };

      if (_params.count(name) && !libmesh_map_find(_params, name)._deprecation_message.empty())
        emit_deprecation_message(name,
                                 "The parameter '" + name + "' is deprecated.\n" +
                                     libmesh_map_find(_params, name)._deprecation_message);
      else if (auto it = _old_to_new_name_and_dep.find(name_in);
               it != _old_to_new_name_and_dep.end() && !it->second.second.empty())
        emit_deprecation_message(name_in, it->second.second);
    }
  }
}

std::string
InputParameters::getClassDescription() const
{
  return _class_description;
}

InputParameters &
InputParameters::operator=(const InputParameters & rhs)
{
  // An error to help minimize the segmentation faults that occure when MooseObjects do not have the
  // correct constructor
  if (!rhs._allow_copy)
  {
    const std::string & name =
        rhs.get<std::string>("_object_name"); // If _allow_parameter_copy is set then so is name
                                              // (see InputParameterWarehouse::addInputParameters)
    mooseError("Copying of the InputParameters object for the ",
               name,
               " object is not allowed.\n\nThe likely cause for this error ",
               "is having a constructor that does not use a const reference, all constructors\nfor "
               "MooseObject based classes should be as follows:\n\n",
               "    MyObject::MyObject(const InputParameters & parameters);");
  }

  Parameters::operator=(rhs);

  _params = rhs._params;

  _buildable_types = rhs._buildable_types;
  _buildable_rm_types = rhs._buildable_rm_types;
  _collapse_nesting = rhs._collapse_nesting;
  _moose_object_syntax_visibility = rhs._moose_object_syntax_visibility;
  _coupled_vars = rhs._coupled_vars;
  _new_to_deprecated_coupled_vars = rhs._new_to_deprecated_coupled_vars;
  _allow_copy = rhs._allow_copy;
  _block_fullpath = rhs._block_fullpath;
  _block_location = rhs._block_location;
  _old_to_new_name_and_dep = rhs._old_to_new_name_and_dep;
  _new_to_old_names = rhs._new_to_old_names;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters & rhs)
{
  Parameters::operator+=(rhs);

  // TODO: this is not a proper merge - if a particular parameter exists in both this and rhs,
  // then we should actually smartly merge both metadata structs before storing in this.
  for (auto it = rhs._params.begin(); it != rhs._params.end(); ++it)
    _params[it->first] = it->second;

  _buildable_types.insert(
      _buildable_types.end(), rhs._buildable_types.begin(), rhs._buildable_types.end());
  _buildable_rm_types.insert(
      _buildable_rm_types.end(), rhs._buildable_rm_types.begin(), rhs._buildable_rm_types.end());

  // Collapse nesting and moose object syntax hiding are not modified with +=
  _coupled_vars.insert(rhs._coupled_vars.begin(), rhs._coupled_vars.end());
  _new_to_deprecated_coupled_vars.insert(rhs._new_to_deprecated_coupled_vars.begin(),
                                         rhs._new_to_deprecated_coupled_vars.end());

  _old_to_new_name_and_dep.insert(rhs._old_to_new_name_and_dep.begin(),
                                  rhs._old_to_new_name_and_dep.end());
  _new_to_old_names.insert(rhs._new_to_old_names.begin(), rhs._new_to_old_names.end());
  return *this;
}

void
InputParameters::setDeprecatedVarDocString(const std::string & new_name,
                                           const std::string & doc_string)
{
  auto coupled_vars_it = _new_to_deprecated_coupled_vars.find(new_name);
  if (coupled_vars_it != _new_to_deprecated_coupled_vars.end())
  {
    auto params_it = _params.find(coupled_vars_it->second);
    if (params_it == _params.end())
      mooseError("There must have been a mistake in the construction of the new to deprecated "
                 "coupled vars map because the old name ",
                 coupled_vars_it->second,
                 " doesn't exist in the parameters data.");

    params_it->second._doc_string = doc_string;
  }
}

void
InputParameters::addCoupledVar(const std::string & name, Real value, const std::string & doc_string)
{
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
  _params[name]._coupled_default.assign(1, value);
  _params[name]._have_coupled_default = true;

  // Set the doc string for any associated deprecated coupled var
  setDeprecatedVarDocString(name, doc_string);
}

void
InputParameters::addCoupledVar(const std::string & name,
                               const std::vector<Real> & value,
                               const std::string & doc_string)
{
  // std::vector<VariableName>(1, Moose::stringify(value)),
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
  _params[name]._coupled_default = value;
  _params[name]._have_coupled_default = true;

  // Set the doc string for any associated deprecated coupled var
  setDeprecatedVarDocString(name, doc_string);
}

void
InputParameters::addCoupledVar(const std::string & name, const std::string & doc_string)
{
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);

  // Set the doc string for any associated deprecated coupled var
  setDeprecatedVarDocString(name, doc_string);
}

void
InputParameters::addDeprecatedCoupledVar(const std::string & old_name,
                                         const std::string & new_name,
                                         const std::string & removal_date /*=""*/)
{
  mooseDeprecated("Please use 'deprecateCoupledVar'");

  _show_deprecated_message = false;

  // Set the doc string if we are adding the deprecated var after the new var has already been added
  auto params_it = _params.find(new_name);
  std::string doc_string;
  if (params_it != _params.end())
    doc_string = params_it->second._doc_string;

  addParam<std::vector<VariableName>>(old_name, doc_string);
  _coupled_vars.insert(old_name);
  _new_to_deprecated_coupled_vars.emplace(new_name, old_name);

  std::string deprecation_message =
      "The coupled variable parameter '" + old_name + "' has been deprecated";
  if (!removal_date.empty())
    deprecation_message += " and will be removed " + removal_date;
  deprecation_message += ". Please use the '" + new_name + "' coupled variable parameter instead.";
  _params[old_name]._deprecation_message = deprecation_message;
  _show_deprecated_message = true;
}

void
InputParameters::addCoupledVarWithAutoBuild(const std::string & name,
                                            const std::string & base_name,
                                            const std::string & num_name,
                                            const std::string & doc_string)
{
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
  _params[name]._autobuild_vecs = std::make_pair(base_name, num_name);

  // Additionally there are two more parameters that need to be added:
  addParam<std::string>(base_name, doc_string + " (base_name)");
  addParam<unsigned int>(num_name, doc_string + " (num_name)");
}

void
InputParameters::addRequiredCoupledVarWithAutoBuild(const std::string & name,
                                                    const std::string & base_name,
                                                    const std::string & num_name,
                                                    const std::string & doc_string)
{
  addRequiredParam<std::vector<VariableName>>(name, doc_string);

  addCoupledVarWithAutoBuild(name, base_name, num_name, doc_string);
}

void
InputParameters::addRequiredCoupledVar(const std::string & name, const std::string & doc_string)
{
  addRequiredParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
}

std::string
InputParameters::getDocString(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);

  std::string doc_string;
  auto it = _params.find(name);
  if (it != _params.end())
    for (const auto & ch : it->second._doc_string)
    {
      if (ch == '\n')
        doc_string += " ... ";
      else
        doc_string += ch;
    }

  return doc_string;
}

void
InputParameters::setDocString(const std::string & name_in, const std::string & doc)
{
  const auto name = checkForRename(name_in);

  auto it = _params.find(name);
  if (it == _params.end())
    mooseError("Unable to set the documentation string (using setDocString) for the \"",
               name,
               "\" parameter, the parameter does not exist.");
  it->second._doc_string = doc;
}

bool
InputParameters::isParamRequired(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return _params.count(name) > 0 && _params.at(name)._required;
}

bool
InputParameters::isParamValid(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  if (have_parameter<MooseEnum>(name))
    return get<MooseEnum>(name).isValid();
  else if (have_parameter<std::vector<MooseEnum>>(name))
  {
    for (auto it = get<std::vector<MooseEnum>>(name).begin();
         it != get<std::vector<MooseEnum>>(name).end();
         ++it)
      if (!it->isValid())
        return false;
    return true;
  }
  else if (have_parameter<MultiMooseEnum>(name))
    return get<MultiMooseEnum>(name).isValid();
  else if (have_parameter<ExecFlagEnum>(name))
    return get<ExecFlagEnum>(name).isValid();
  else
    return _params.count(name) > 0 && _params.at(name)._valid;
}

bool
InputParameters::isParamSetByAddParam(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return _params.count(name) > 0 && _params.at(name)._set_by_add_param;
}

bool
InputParameters::isParamDeprecated(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return _params.count(name) > 0 && !_params.at(name)._deprecation_message.empty();
}

bool
InputParameters::areAllRequiredParamsValid() const
{
  for (const auto & it : *this)
    if (isParamRequired(it.first) && !isParamValid(it.first))
      return false;
  return true;
}

bool
InputParameters::isPrivate(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return _params.count(name) > 0 && _params.at(name)._is_private;
}

void
InputParameters::declareControllable(const std::string & input_names,
                                     std::set<ExecFlagType> execute_flags)
{
  std::vector<std::string> names;
  MooseUtils::tokenize<std::string>(input_names, names, 1, " ");
  for (auto & name_in : names)
  {
    const auto name = checkForRename(name_in);
    auto map_iter = _params.find(name);
    if (map_iter != _params.end()) // error is handled by checkParams method
    {
      map_iter->second._controllable = true;
      map_iter->second._controllable_flags = execute_flags;
    }
    else
      mooseError("The input parameter '",
                 name,
                 "' does not exist, thus cannot be marked as controllable.");
  }
}

bool
InputParameters::isControllable(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return _params.count(name) > 0 && _params.at(name)._controllable;
}

const std::set<ExecFlagType> &
InputParameters::getControllableExecuteOnTypes(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  return at(name)._controllable_flags;
}

void
InputParameters::registerBase(const std::string & value)
{
  InputParameters::set<std::string>("_moose_base") = value;
  _params["_moose_base"]._is_private = true;
}

void
InputParameters::registerSystemAttributeName(const std::string & value)
{
  InputParameters::set<std::string>("_moose_warehouse_system_name") = value;
  _params["_moose_warehouse_system_name"]._is_private = true;
}

void
InputParameters::registerBuildableTypes(const std::string & names)
{
  _buildable_types.clear();
  MooseUtils::tokenize(names, _buildable_types, 1, " \t\n\v\f\r"); // tokenize on whitespace
}

void
InputParameters::addRelationshipManager(
    const std::string & name,
    Moose::RelationshipManagerType rm_type,
    Moose::RelationshipManagerInputParameterCallback input_parameter_callback)
{
  _buildable_rm_types.emplace_back(name, rm_type, input_parameter_callback);
}

const std::vector<std::string> &
InputParameters::getBuildableTypes() const
{
  return _buildable_types;
}

const std::vector<std::tuple<std::string,
                             Moose::RelationshipManagerType,
                             Moose::RelationshipManagerInputParameterCallback>> &
InputParameters::getBuildableRelationshipManagerTypes() const
{
  return _buildable_rm_types;
}

void
InputParameters::collapseSyntaxNesting(bool collapse)
{
  _collapse_nesting = collapse;
}

bool
InputParameters::collapseSyntaxNesting() const
{
  return _collapse_nesting;
}

void
InputParameters::mooseObjectSyntaxVisibility(bool visibility)
{
  _moose_object_syntax_visibility = visibility;
}

bool
InputParameters::mooseObjectSyntaxVisibility() const
{
  return _moose_object_syntax_visibility;
}

#define dynamicCastRangeCheck(type, up_type, long_name, short_name, param, oss)                    \
  do                                                                                               \
  {                                                                                                \
    libMesh::Parameters::Value * val = MooseUtils::get(param);                                     \
    InputParameters::Parameter<type> * scalar_p =                                                  \
        dynamic_cast<InputParameters::Parameter<type> *>(val);                                     \
    if (scalar_p)                                                                                  \
      rangeCheck<type, up_type>(long_name, short_name, scalar_p, oss);                             \
    InputParameters::Parameter<std::vector<type>> * vector_p =                                     \
        dynamic_cast<InputParameters::Parameter<std::vector<type>> *>(val);                        \
    if (vector_p)                                                                                  \
      rangeCheck<type, up_type>(long_name, short_name, vector_p, oss);                             \
  } while (0)

#define checkMooseType(param_type, name)                                                           \
  if (have_parameter<param_type>(name) || have_parameter<std::vector<param_type>>(name))           \
    oss << inputLocation(param_name) << ": non-controllable type '" << type(name)                  \
        << "' for parameter '" << paramFullpath(param_name) << "' marked controllable";

void
InputParameters::checkParams(const std::string & parsing_syntax)
{
  std::string parampath = blockFullpath() != "" ? blockFullpath() : parsing_syntax;
  std::ostringstream oss;
  // Required parameters
  for (const auto & it : *this)
  {
    const auto param_name = checkForRename(it.first);
    if (!isParamValid(param_name) && isParamRequired(param_name))
    {
      // check if an old, deprecated name exists for this parameter that may be specified
      auto oit = _new_to_deprecated_coupled_vars.find(param_name);
      if (oit != _new_to_deprecated_coupled_vars.end() && isParamValid(oit->second))
        continue;

      oss << blockLocation() << ": missing required parameter '" << parampath + "/" + param_name
          << "'\n";
      oss << "\tDoc String: \"" + getDocString(param_name) + "\"" << std::endl;
    }
  }

  // Range checked parameters
  for (const auto & it : *this)
  {
    std::string long_name(parampath + "/" + it.first);

    dynamicCastRangeCheck(Real, Real, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(int, long, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(long, long, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(unsigned int, long, long_name, it.first, it.second, oss);
  }

  // Controllable parameters
  for (const auto & param_name : getControllableParameters())
  {
    if (isPrivate(param_name))
      oss << inputLocation(param_name) << ": private parameter '" << paramFullpath(param_name)
          << "' marked controllable";

    checkMooseType(NonlinearVariableName, param_name);
    checkMooseType(AuxVariableName, param_name);
    checkMooseType(VariableName, param_name);
    checkMooseType(BoundaryName, param_name);
    checkMooseType(SubdomainName, param_name);
    checkMooseType(PostprocessorName, param_name);
    checkMooseType(VectorPostprocessorName, param_name);
    checkMooseType(UserObjectName, param_name);
    checkMooseType(MaterialPropertyName, param_name);
  }

  if (!oss.str().empty())
    mooseError(oss.str());
}

bool
InputParameters::hasCoupledValue(const std::string & coupling_name) const
{
  return _coupled_vars.find(coupling_name) != _coupled_vars.end();
}

bool
InputParameters::hasDefaultCoupledValue(const std::string & coupling_name) const
{
  return _params.count(coupling_name) > 0 && _params.at(coupling_name)._have_coupled_default &&
         _coupled_vars.count(coupling_name) > 0;
}

void
InputParameters::defaultCoupledValue(const std::string & coupling_name, Real value, unsigned int i)
{
  _params[coupling_name]._coupled_default.resize(i + 1);
  _params[coupling_name]._coupled_default[i] = value;
  _params[coupling_name]._have_coupled_default = true;
}

Real
InputParameters::defaultCoupledValue(const std::string & coupling_name, unsigned int i) const
{
  auto value_it = _params.find(coupling_name);

  if (value_it == _params.end() || !value_it->second._have_coupled_default)
    mooseError("Attempted to retrieve default value for coupled variable '",
               coupling_name,
               "' when none was provided. \n\nThere are three reasons why this may have "
               "occurred:\n 1. The other version of params.addCoupledVar() should be used in order "
               "to provide a default value. \n 2. This should have been a required coupled "
               "variable added with params.addRequiredCoupledVar() \n 3. The call to get the "
               "coupled value should have been properly guarded with isCoupled()\n");

  return value_it->second._coupled_default.at(i);
}

unsigned int
InputParameters::numberDefaultCoupledValues(const std::string & coupling_name) const
{
  auto value_it = _params.find(coupling_name);
  if (value_it == _params.end())
    mooseError("Attempted to retrieve default value for coupled variable '",
               coupling_name,
               "' when none was provided.");
  return value_it->second._coupled_default.size();
}

std::map<std::string, std::pair<std::string, std::string>>
InputParameters::getAutoBuildVectors() const
{
  std::map<std::string, std::pair<std::string, std::string>> abv;
  for (auto it = _params.begin(); it != _params.end(); ++it)
  {
    if (!it->second._autobuild_vecs.first.empty())
      abv[it->first] = it->second._autobuild_vecs;
  }
  return abv;
}

std::string
InputParameters::type(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  if (!_values.count(name))
    mooseError("Parameter \"", name, "\" not found.\n\n", *this);

  if (_coupled_vars.find(name) != _coupled_vars.end())
    return "std::vector<VariableName>";
  else if (_params.count(name) > 0 && !_params.at(name)._custom_type.empty())
    return _params.at(name)._custom_type;
  return _values.at(name)->type();
}

std::string
InputParameters::getMooseType(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  std::string var;

  if (have_parameter<VariableName>(name))
    var = get<VariableName>(name);
  else if (have_parameter<NonlinearVariableName>(name))
    var = get<NonlinearVariableName>(name);
  else if (have_parameter<AuxVariableName>(name))
    var = get<AuxVariableName>(name);
  else if (have_parameter<PostprocessorName>(name))
    var = get<PostprocessorName>(name);
  else if (have_parameter<VectorPostprocessorName>(name))
    var = get<VectorPostprocessorName>(name);
  else if (have_parameter<FunctionName>(name))
    var = get<FunctionName>(name);
  else if (have_parameter<UserObjectName>(name))
    var = get<UserObjectName>(name);
  else if (have_parameter<MaterialPropertyName>(name))
    var = get<MaterialPropertyName>(name);
  else if (have_parameter<std::string>(name))
    var = get<std::string>(name);

  return var;
}

std::vector<std::string>
InputParameters::getVecMooseType(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  std::vector<std::string> svars;

  if (have_parameter<std::vector<VariableName>>(name))
  {
    std::vector<VariableName> vars = get<std::vector<VariableName>>(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<NonlinearVariableName>>(name))
  {
    std::vector<NonlinearVariableName> vars = get<std::vector<NonlinearVariableName>>(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<AuxVariableName>>(name))
  {
    std::vector<AuxVariableName> vars = get<std::vector<AuxVariableName>>(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<MaterialPropertyName>>(name))
  {
    std::vector<MaterialPropertyName> vars = get<std::vector<MaterialPropertyName>>(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<std::string>>(name))
  {
    std::vector<std::string> vars = get<std::vector<std::string>>(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }

  return svars;
}

void
InputParameters::addParamNamesToGroup(const std::string & space_delim_names,
                                      const std::string group_name)
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(space_delim_names, elements, 1, " \t\n\v\f\r"); // tokenize on whitespace

  // Since we don't require types (templates) for this method, we need
  // to get a raw list of parameter names to compare against.
  std::set<std::string> param_names;
  for (const auto & it : *this)
    param_names.insert(it.first);

  for (const auto & param_name : elements)
    if (_params.count(param_name) > 0)
      _params[param_name]._group = group_name;
    else
      mooseError("Unable to find a parameter with name: ",
                 param_name,
                 " when adding to group ",
                 group_name,
                 '.');
}

std::vector<std::string>
InputParameters::getSyntax(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  auto it = _params.find(name);
  if (it == _params.end())
    mooseError("No parameter exists with the name ", name);
  return it->second._cli_flag_names;
}

std::string
InputParameters::getGroupName(const std::string & param_name_in) const
{
  const auto param_name = checkForRename(param_name_in);
  auto it = _params.find(param_name);
  if (it != _params.end())
    return it->second._group;
  return std::string();
}

void
InputParameters::applyParameters(const InputParameters & common,
                                 const std::vector<std::string> exclude)
{
  // Loop through the common parameters
  for (const auto & it : common)
  {
    // Common parameter name
    const std::string & common_name = it.first;

    // Continue to next parameter, if the current is in list of  excluded parameters
    if (std::find(exclude.begin(), exclude.end(), common_name) != exclude.end())
      continue;

    applyParameter(common, common_name);
  }

  // Loop through the coupled variables
  for (std::set<std::string>::const_iterator it = common.coupledVarsBegin();
       it != common.coupledVarsEnd();
       ++it)
  {
    // Variable name
    const std::string var_name = *it;

    // Continue to next variable, if the current is in list of  excluded parameters
    if (std::find(exclude.begin(), exclude.end(), var_name) != exclude.end())
      continue;

    applyCoupledVar(common, var_name);
  }
}

void
InputParameters::applySpecificParameters(const InputParameters & common,
                                         const std::vector<std::string> & include,
                                         bool allow_private)
{
  // Loop through the common parameters
  for (const auto & it : common)
  {

    // Common parameter name
    const std::string & common_name = it.first;

    // Continue to next parameter, if the current is not in list of included parameters
    if (std::find(include.begin(), include.end(), common_name) == include.end())
      continue;

    applyParameter(common, common_name, allow_private);
  }

  // Loop through the coupled variables
  for (std::set<std::string>::const_iterator it = common.coupledVarsBegin();
       it != common.coupledVarsEnd();
       ++it)
  {
    // Variable name
    const std::string var_name = *it;

    // Continue to next variable, if the current is not in list of included parameters
    if (std::find(include.begin(), include.end(), var_name) == include.end())
      continue;

    applyCoupledVar(common, var_name);
  }
}

void
InputParameters::applyCoupledVar(const InputParameters & common, const std::string & var_name)
{
  // Disable the display of deprecated message when applying common parameters, this avoids a dump
  // of messages
  _show_deprecated_message = false;

  // If the local parameters has a coupled variable, populate it with the value from the common
  // parameters, if the common parameters has the coupled variable too
  if (hasCoupledValue(var_name))
  {
    if (common.hasDefaultCoupledValue(var_name))
    {
      // prepare a vector of default coupled values
      std::vector<Real> defaults(common.numberDefaultCoupledValues(var_name));
      for (unsigned int j = 0; j < common.numberDefaultCoupledValues(var_name); ++j)
        defaults[j] = common.defaultCoupledValue(var_name, j);
      addCoupledVar(var_name, defaults, common.getDocString(var_name));
    }
    else if (common.hasCoupledValue(var_name))
      addCoupledVar(var_name, common.getDocString(var_name));
  }

  // Enable deprecated message printing
  _show_deprecated_message = true;
}

void
InputParameters::applyParameter(const InputParameters & common,
                                const std::string & common_name,
                                bool allow_private)
{
  // Disable the display of deprecated message when applying common parameters, this avoids a dump
  // of messages
  _show_deprecated_message = false;

  const auto local_name = checkForRename(common_name);

  // Extract the properties from the local parameter for the current common parameter name
  const bool local_exist = _values.find(local_name) != _values.end();
  const bool local_set = _params.count(local_name) > 0 && !_params[local_name]._set_by_add_param;
  const bool local_priv = allow_private ? false : isPrivate(local_name);
  const bool local_valid = isParamValid(local_name);

  // Extract the properties from the common parameter
  const bool common_exist = common._values.find(common_name) != common._values.end();
  const bool common_priv = allow_private ? false : common.isPrivate(common_name);
  const bool common_valid = common.isParamValid(common_name);

  /* In order to apply common parameter 4 statements must be satisfied
   * (1) A local parameter must exist with the same name as common parameter
   * (2) Common parameter must valid and exist
   * (3) Local parameter must be invalid OR not have been set from its default
   * (4) Both cannot be private
   */
  if (local_exist && common_exist && common_valid && (!local_valid || !local_set) &&
      (!common_priv || !local_priv))
  {
    remove(local_name);
    _values[local_name] = common._values.find(common_name)->second->clone();
    set_attributes(local_name, false);
    _params[local_name]._set_by_add_param =
        libmesh_map_find(common._params, common_name)._set_by_add_param;
  }

  // Enable deprecated message printing
  _show_deprecated_message = true;
}

///Deprecated method
bool
InputParameters::paramSetByUser(const std::string & name) const
{
  mooseDeprecated("paramSetByUser() is deprecated. Use isParamSetByUser() instead.");
  return isParamSetByUser(name);
}

bool
InputParameters::isParamSetByUser(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  if (!isParamValid(name))
    // if the parameter is invalid, it is for sure not set by the user
    return false;
  else
    // If the parameters is not located in the list, then it was set by the user
    // If the parameter is private, and present in global params, it is ignored, therefore not set
    return _params.count(name) > 0 && !_params.at(name)._set_by_add_param &&
           !_params.at(name)._is_private;
}

const std::string &
InputParameters::getDescription(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  auto it = _params.find(name);
  if (it == _params.end())
    mooseError("No parameter exists with the name ", name);
  return it->second._doc_string;
}

template <>
void
InputParameters::addRequiredParam<MooseEnum>(const std::string & name,
                                             const MooseEnum & moose_enum,
                                             const std::string & doc_string)
{
  InputParameters::set<MooseEnum>(name) = moose_enum; // valid parameter is set by set_attributes
  _params[name]._required = true;
  _params[name]._doc_string = doc_string;
}

template <>
void
InputParameters::addRequiredParam<MultiMooseEnum>(const std::string & name,
                                                  const MultiMooseEnum & moose_enum,
                                                  const std::string & doc_string)
{
  InputParameters::set<MultiMooseEnum>(name) =
      moose_enum; // valid parameter is set by set_attributes
  _params[name]._required = true;
  _params[name]._doc_string = doc_string;
}

template <>
void
InputParameters::addRequiredParam<std::vector<MooseEnum>>(
    const std::string & name,
    const std::vector<MooseEnum> & moose_enums,
    const std::string & doc_string)
{
  InputParameters::set<std::vector<MooseEnum>>(name) =
      moose_enums; // valid parameter is set by set_attributes
  _params[name]._required = true;
  _params[name]._doc_string = doc_string;
}

template <>
void
InputParameters::addParam<MooseEnum>(const std::string & /*name*/,
                                     const std::string & /*doc_string*/)
{
  mooseError("You must supply a MooseEnum object when using addParam, even if the parameter is not "
             "required!");
}

template <>
void
InputParameters::addParam<MultiMooseEnum>(const std::string & /*name*/,
                                          const std::string & /*doc_string*/)
{
  mooseError("You must supply a MultiMooseEnum object when using addParam, even if the parameter "
             "is not required!");
}

template <>
void
InputParameters::addParam<std::vector<MooseEnum>>(const std::string & /*name*/,
                                                  const std::string & /*doc_string*/)
{
  mooseError("You must supply a vector of MooseEnum object(s) when using addParam, even if the "
             "parameter is not required!");
}

template <>
void
InputParameters::addPrivateParam<MooseEnum>(const std::string & /*name*/)
{
  mooseError("You must supply a MooseEnum object when using addPrivateParam, even if the parameter "
             "is not required!");
}

template <>
void
InputParameters::addPrivateParam<MultiMooseEnum>(const std::string & /*name*/)
{
  mooseError("You must supply a MultiMooseEnum object when using addPrivateParam, even if the "
             "parameter is not required!");
}

template <>
void
InputParameters::addDeprecatedParam<MooseEnum>(const std::string & /*name*/,
                                               const std::string & /*doc_string*/,
                                               const std::string & /*deprecation_message*/)
{
  mooseError("You must supply a MooseEnum object and the deprecation string when using "
             "addDeprecatedParam, even if the parameter is not required!");
}

template <>
void
InputParameters::addDeprecatedParam<MultiMooseEnum>(const std::string & /*name*/,
                                                    const std::string & /*doc_string*/,
                                                    const std::string & /*deprecation_message*/)
{
  mooseError("You must supply a MultiMooseEnum object and the deprecation string when using "
             "addDeprecatedParam, even if the parameter is not required!");
}

template <>
void
InputParameters::addDeprecatedParam<std::vector<MooseEnum>>(
    const std::string & /*name*/,
    const std::string & /*doc_string*/,
    const std::string & /*deprecation_message*/)
{
  mooseError("You must supply a vector of MooseEnum object(s) and the deprecation string when "
             "using addDeprecatedParam, even if the parameter is not required!");
}

template <>
void
InputParameters::setParamHelper<PostprocessorName, Real>(const std::string & /*name*/,
                                                         PostprocessorName & l_value,
                                                         const Real & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<PostprocessorName, int>(const std::string & /*name*/,
                                                        PostprocessorName & l_value,
                                                        const int & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<FunctionName, Real>(const std::string & /*name*/,
                                                    FunctionName & l_value,
                                                    const Real & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<FunctionName, int>(const std::string & /*name*/,
                                                   FunctionName & l_value,
                                                   const int & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<MaterialPropertyName, Real>(const std::string & /*name*/,
                                                            MaterialPropertyName & l_value,
                                                            const Real & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<MaterialPropertyName, int>(const std::string & /*name*/,
                                                           MaterialPropertyName & l_value,
                                                           const int & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<MooseFunctorName, Real>(const std::string & /*name*/,
                                                        MooseFunctorName & l_value,
                                                        const Real & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<MooseFunctorName, int>(const std::string & /*name*/,
                                                       MooseFunctorName & l_value,
                                                       const int & r_value)
{
  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
const MooseEnum &
InputParameters::getParamHelper<MooseEnum>(const std::string & name_in,
                                           const InputParameters & pars,
                                           const MooseEnum *)
{
  const auto name = pars.checkForRename(name_in);
  return pars.get<MooseEnum>(name);
}

template <>
const MultiMooseEnum &
InputParameters::getParamHelper<MultiMooseEnum>(const std::string & name_in,
                                                const InputParameters & pars,
                                                const MultiMooseEnum *)
{
  const auto name = pars.checkForRename(name_in);
  return pars.get<MultiMooseEnum>(name);
}

void
InputParameters::setReservedValues(const std::string & name_in,
                                   const std::set<std::string> & reserved)
{
  const auto name = checkForRename(name_in);
  _params[name]._reserved_values = reserved;
}

std::set<std::string>
InputParameters::reservedValues(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);
  auto it = _params.find(name);
  if (it == _params.end())
    return std::set<std::string>();
  return it->second._reserved_values;
}

void
InputParameters::checkParamName(const std::string & name) const
{
  const static pcrecpp::RE valid("[\\w:/]+");
  if (!valid.FullMatch(name))
    mooseError("Invalid parameter name: '", name, "'");
}

bool
InputParameters::shouldIgnore(const std::string & name_in)
{
  const auto name = checkForRename(name_in);
  auto it = _params.find(name);
  if (it != _params.end())
    return it->second._ignore;
  mooseError("Parameter ", name, " does not exist");
}

std::set<std::string>
InputParameters::getGroupParameters(const std::string & group) const
{
  std::set<std::string> names;
  for (auto it = _params.begin(); it != _params.end(); ++it)
    if (it->second._group == group)
      names.emplace(it->first);
  return names;
}

std::set<std::string>
InputParameters::getControllableParameters() const
{
  std::set<std::string> controllable;
  for (auto it = _params.begin(); it != _params.end(); ++it)
    if (it->second._controllable)
      controllable.emplace(it->first);
  return controllable;
}

std::string
InputParameters::errorPrefix(const std::string & param) const
{
  auto prefix = param + ":";
  if (!inputLocation(param).empty())
    prefix = inputLocation(param) + ": (" + paramFullpath(param) + ")";
  return prefix;
}

std::string
InputParameters::varName(const std::string & var_param_name,
                         const std::string & moose_object_with_var_param_name) const
{
  // Try the scalar version first
  std::string variable_name = getMooseType(var_param_name);
  if (variable_name == "")
  {
    auto vec = getVecMooseType(var_param_name);

    // Catch the (very unlikely) case where a user specifies
    // variable = '' (the empty string)
    // in their input file. This could happen if e.g. something goes
    // wrong with dollar bracket expression expansion.
    if (vec.empty())
      mooseError("Error constructing object '",
                 moose_object_with_var_param_name,
                 "' while retrieving value for '",
                 var_param_name,
                 "' parameter! Did you forget to set '",
                 var_param_name,
                 "' or set it to '' (empty string) by accident?");

    // When using vector variables, we are only going to use the first one in the list at the
    // interface level...
    variable_name = vec[0];
  }

  return variable_name;
}

void
InputParameters::renameParamInternal(const std::string & old_name,
                                     const std::string & new_name,
                                     const std::string & docstring,
                                     const std::string & removal_date)
{
  auto params_it = _params.find(old_name);
  if (params_it == _params.end())
    mooseError("Requested to rename parameter '",
               old_name,
               "' but that parameter name doesn't exist in the parameters object.");

  auto new_metadata = std::move(params_it->second);
  if (!docstring.empty())
    new_metadata._doc_string = docstring;
  _params.emplace(new_name, std::move(new_metadata));
  _params.erase(params_it);

  auto values_it = _values.find(old_name);
  auto new_value = std::move(values_it->second);
  _values.emplace(new_name, std::move(new_value));
  _values.erase(values_it);

  std::string deprecation_message;
  if (!removal_date.empty())
    deprecation_message = "'" + old_name + "' has been deprecated and will be removed on " +
                          removal_date + ". Please use '" + new_name + "' instead.";

  _old_to_new_name_and_dep.emplace(old_name, std::make_pair(new_name, deprecation_message));
  _new_to_old_names.emplace(new_name, old_name);
}

void
InputParameters::renameCoupledVarInternal(const std::string & old_name,
                                          const std::string & new_name,
                                          const std::string & docstring,
                                          const std::string & removal_date)
{
  auto coupled_vars_it = _coupled_vars.find(old_name);
  if (coupled_vars_it == _coupled_vars.end())
    mooseError("Requested to rename coupled variable '",
               old_name,
               "' but that coupled variable name doesn't exist in the parameters object.");

  _coupled_vars.insert(new_name);
  _coupled_vars.erase(coupled_vars_it);

  renameParamInternal(old_name, new_name, docstring, removal_date);
}

void
InputParameters::renameParam(const std::string & old_name,
                             const std::string & new_name,
                             const std::string & new_docstring)
{
  renameParamInternal(old_name, new_name, new_docstring, "");
}

void
InputParameters::renameCoupledVar(const std::string & old_name,
                                  const std::string & new_name,
                                  const std::string & new_docstring)
{
  renameCoupledVarInternal(old_name, new_name, new_docstring, "");
}

void
InputParameters::deprecateParam(const std::string & old_name,
                                const std::string & new_name,
                                const std::string & removal_date)
{
  renameParamInternal(old_name, new_name, "", removal_date);
}

void
InputParameters::deprecateCoupledVar(const std::string & old_name,
                                     const std::string & new_name,
                                     const std::string & removal_date)
{
  renameCoupledVarInternal(old_name, new_name, "", removal_date);
}

std::string
InputParameters::checkForRename(const std::string & name) const
{
  if (auto it = _old_to_new_name_and_dep.find(name); it != _old_to_new_name_and_dep.end())
    return it->second.first;
  else
    return name;
}

std::vector<std::string>
InputParameters::paramAliases(const std::string & param_name) const
{
  mooseAssert(_values.find(param_name) != _values.end(),
              "The parameter we are searching for aliases for should exist in our parameter map");
  std::vector<std::string> aliases = {param_name};

  for (const auto & pr : as_range(_new_to_old_names.equal_range(param_name)))
    aliases.push_back(pr.second);

  return aliases;
}
