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
  _collapse_nesting = false;
  _moose_object_syntax_visibility = true;
  _show_deprecated_message = true;
  _allow_copy = true;
  _block_fullpath = "";
  _block_location = "";
}

void
InputParameters::addClassDescription(const std::string & doc_string)
{
  _class_description = doc_string;
}

void
InputParameters::set_attributes(const std::string & name, bool inserted_only)
{
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
      if (_params.count(name) && !_params[name]._deprecation_message.empty())
        mooseDeprecated(
            "The parameter ", name, " is deprecated.\n", _params[name]._deprecation_message);
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
  _allow_copy = rhs._allow_copy;
  _block_fullpath = rhs._block_fullpath;
  _block_location = rhs._block_location;

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
  return *this;
}

void
InputParameters::addCoupledVar(const std::string & name, Real value, const std::string & doc_string)
{
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
  _params[name]._coupled_default.assign(1, value);
  _params[name]._have_coupled_default = true;
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
}

void
InputParameters::addCoupledVar(const std::string & name, const std::string & doc_string)
{
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
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
InputParameters::getDocString(const std::string & name) const
{
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
InputParameters::setDocString(const std::string & name, const std::string & doc)
{
  auto it = _params.find(name);
  if (it == _params.end())
    mooseError("Unable to set the documentation string (using setDocString) for the \"",
               name,
               "\" parameter, the parameter does not exist.");
  it->second._doc_string = doc;
}

bool
InputParameters::isParamRequired(const std::string & name) const
{
  return _params.count(name) > 0 && _params.at(name)._required;
}

bool
InputParameters::isParamValid(const std::string & name) const
{
  if (have_parameter<MooseEnum>(name))
    return get<MooseEnum>(name).isValid();
  else if (have_parameter<MultiMooseEnum>(name))
    return get<MultiMooseEnum>(name).isValid();
  else if (have_parameter<ExecFlagEnum>(name))
    return get<ExecFlagEnum>(name).isValid();
  else
    return _params.count(name) > 0 && _params.at(name)._valid;
}

bool
InputParameters::isParamSetByAddParam(const std::string & name) const
{
  return _params.count(name) > 0 && _params.at(name)._set_by_add_param;
}

bool
InputParameters::isParamDeprecated(const std::string & name) const
{
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
InputParameters::isPrivate(const std::string & name) const
{
  return _params.count(name) > 0 && _params.at(name)._is_private;
}

void
InputParameters::declareControllable(const std::string & input_names)
{
  std::vector<std::string> names;
  MooseUtils::tokenize<std::string>(input_names, names, 1, " ");
  for (auto & name : names)
  {
    auto map_iter = _params.find(name);
    if (map_iter != _params.end()) // error is handled by checkParams method
      map_iter->second._controllable = true;
    else
      mooseError("The input parameter '",
                 name,
                 "' does not exist, thus cannot be marked as controllable.");
  }
}

bool
InputParameters::isControllable(const std::string & name)
{
  return _params.count(name) > 0 && _params[name]._controllable;
}

void
InputParameters::registerBase(const std::string & value)
{
  InputParameters::set<std::string>("_moose_base") = value;
  _params["_moose_base"]._is_private = true;
}

void
InputParameters::registerBuildableTypes(const std::string & names)
{
  _buildable_types.clear();
  MooseUtils::tokenize(names, _buildable_types, 1, " \t\n\v\f\r"); // tokenize on whitespace
}

void
InputParameters::registerRelationshipManagers(const std::string & names,
                                              const std::string & use_as_rm_types)
{
  _buildable_rm_types.clear();

  std::vector<std::string> buildable_rm_types;
  MooseUtils::tokenize(names, buildable_rm_types, 1, " \t\n\v\f\r"); // tokenize on whitespace

  std::vector<std::string> use_as_rm_type_strings;
  MooseUtils::tokenize(use_as_rm_types, use_as_rm_type_strings, 1, " \t\n\v\f\r");

  for (std::size_t i = 0; i < buildable_rm_types.size(); ++i)
  {
    Moose::RelationshipManagerType use_as_type = Moose::RelationshipManagerType::DEFAULT;
    if (i < use_as_rm_type_strings.size())
      use_as_type = Moose::stringToEnum<Moose::RelationshipManagerType>(use_as_rm_type_strings[i]);

    _buildable_rm_types.emplace_back(std::make_pair(buildable_rm_types[i], use_as_type));
  }
}

const std::vector<std::string> &
InputParameters::getBuildableTypes() const
{
  return _buildable_types;
}

const std::vector<std::pair<std::string, Moose::RelationshipManagerType>> &
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
    InputParameters::Parameter<type> * scalar_p =                                                  \
        dynamic_cast<InputParameters::Parameter<type> *>(param);                                   \
    if (scalar_p)                                                                                  \
      rangeCheck<type, up_type>(long_name, short_name, scalar_p, oss);                             \
    InputParameters::Parameter<std::vector<type>> * vector_p =                                     \
        dynamic_cast<InputParameters::Parameter<std::vector<type>> *>(param);                      \
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
    if (!isParamValid(it.first) && isParamRequired(it.first))
    {
      oss << blockLocation() << ": missing required parameter '" << parampath + "/" + it.first
          << "'\n";
      oss << "\tDoc String: \"" + getDocString(it.first) + "\"" << std::endl;
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

  return value_it->second._coupled_default[i];
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
InputParameters::type(const std::string & name)
{
  if (_coupled_vars.find(name) != _coupled_vars.end())
    return "std::vector<VariableName>";
  else if (_params.count(name) > 0 && !_params[name]._custom_type.empty())
    return _params[name]._custom_type;
  return _values[name]->type();
}

std::string
InputParameters::getMooseType(const std::string & name) const
{
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
InputParameters::getVecMooseType(const std::string & name) const
{
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
InputParameters::getSyntax(const std::string & name)
{
  return _params[name]._cli_flag_names;
}

std::string
InputParameters::getGroupName(const std::string & param_name) const
{
  auto it = _params.find(param_name);
  if (it != _params.end())
    return it->second._group;
  return std::string();
}

const PostprocessorValue &
InputParameters::getDefaultPostprocessorValue(const std::string & name, bool suppress_error) const
{
  // Check that a default exists, error if it does not
  auto it = _params.find(name);
  if (!suppress_error && (it == _params.end() || !it->second._have_default_postprocessor_val))
    mooseError("A default PostprcessorValue does not exist for the given name: ", name);

  return it->second._default_postprocessor_val;
}

void
InputParameters::setDefaultPostprocessorValue(const std::string & name,
                                              const PostprocessorValue & value)
{
  _params[name]._default_postprocessor_val = value;
  _params[name]._have_default_postprocessor_val = true;
}

bool
InputParameters::hasDefaultPostprocessorValue(const std::string & name) const
{
  return _params.count(name) > 0 && _params.at(name)._have_default_postprocessor_val;
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
                                         const std::vector<std::string> & include)
{
  // Loop through the common parameters
  for (const auto & it : common)
  {

    // Common parameter name
    const std::string & common_name = it.first;

    // Continue to next parameter, if the current is not in list of included parameters
    if (std::find(include.begin(), include.end(), common_name) == include.end())
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
InputParameters::applyParameter(const InputParameters & common, const std::string & common_name)
{
  // Disable the display of deprecated message when applying common parameters, this avoids a dump
  // of messages
  _show_deprecated_message = false;

  // Extract the properties from the local parameter for the current common parameter name
  const bool local_exist = _values.find(common_name) != _values.end();
  const bool local_set = _params.count(common_name) > 0 && !_params[common_name]._set_by_add_param;
  const bool local_priv = isPrivate(common_name);
  const bool local_valid = isParamValid(common_name);

  // Extract the properties from the common parameter
  const bool common_exist = common._values.find(common_name) != common._values.end();
  const bool common_priv = common.isPrivate(common_name);
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
    delete _values[common_name];
    _values[common_name] = common._values.find(common_name)->second->clone();
    set_attributes(common_name, false);
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
InputParameters::isParamSetByUser(const std::string & name) const
{
  if (!isParamValid(name))
    // if the parameter is invalid, it is for sure not set by the user
    return false;
  else
    // If the parameters is not located in the list, then it was set by the user
    return _params.count(name) > 0 && !_params.at(name)._set_by_add_param;
}

const std::string &
InputParameters::getDescription(const std::string & name)
{
  if (_params.count(name) == 0)
    mooseError("No parameter exists with the name ", name);
  return _params[name]._doc_string;
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
InputParameters::setParamHelper<PostprocessorName, Real>(const std::string & name,
                                                         PostprocessorName & l_value,
                                                         const Real & r_value)
{
  // Store the default value
  _params[name]._default_postprocessor_val = r_value;
  _params[name]._have_default_postprocessor_val = true;

  // Assign the default value so that it appears in the dump
  std::ostringstream oss;
  oss << r_value;
  l_value = oss.str();
}

template <>
void
InputParameters::setParamHelper<PostprocessorName, int>(const std::string & name,
                                                        PostprocessorName & l_value,
                                                        const int & r_value)
{
  // Store the default value
  _params[name]._default_postprocessor_val = r_value;
  _params[name]._have_default_postprocessor_val = true;

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
const MooseEnum &
InputParameters::getParamHelper<MooseEnum>(const std::string & name,
                                           const InputParameters & pars,
                                           const MooseEnum *)
{
  return pars.get<MooseEnum>(name);
}

template <>
const MultiMooseEnum &
InputParameters::getParamHelper<MultiMooseEnum>(const std::string & name,
                                                const InputParameters & pars,
                                                const MultiMooseEnum *)
{
  return pars.get<MultiMooseEnum>(name);
}

void
InputParameters::setReservedValues(const std::string & name, const std::set<std::string> & reserved)
{
  _params[name]._reserved_values = reserved;
}

std::set<std::string>
InputParameters::reservedValues(const std::string & name) const
{
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
