/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseUtils.h"

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
  Parameters::operator=(rhs);
  _collapse_nesting = false;
  _moose_object_syntax_visibility = true;
}

void
InputParameters::clear()
{
  Parameters::clear();
  _doc_string.clear();
  _custom_type.clear();
  _group.clear();
  _range_functions.clear();
  _auto_build_vectors.clear();
  _required_params.clear();
  _valid_params.clear();
  _private_params.clear();
  _coupled_vars.clear();
  _syntax.clear();
  _default_coupled_value.clear();
  _default_postprocessor_value.clear();
  _collapse_nesting = false;
  _moose_object_syntax_visibility = true;
  _show_deprecated_message = true;
  _allow_copy = true;
}

void
InputParameters::addClassDescription(const std::string & doc_string)
{
  _doc_string["_class"] = doc_string;
}

void
InputParameters::set_attributes(const std::string & name, bool inserted_only)
{
  if (!inserted_only)
  {
    /**
     * "_set_by_add_param" and "_deprecated_params" are not populated until after
     * the default value has already been set in libMesh (first callback to this
     * method). Therefore if a variable is in/not in one of these sets, you can
     * be assured it was put there outside of the "addParam*()" calls.
     */
    _set_by_add_param.erase(name);

    // valid_params don't make sense for MooseEnums
    if (!have_parameter<MooseEnum>(name) && !have_parameter<MultiMooseEnum>(name))
      _valid_params.insert(name);

    if (_show_deprecated_message)
    {
      std::map<std::string, std::string>::const_iterator pos = _deprecated_params.find(name);
      if (pos != _deprecated_params.end())
      {
        mooseDeprecated("The parameter ", name, " is deprecated.\n", pos->second);
      }
    }
  }
}

std::string
InputParameters::getClassDescription() const
{
  std::map<std::string, std::string>::const_iterator pos = _doc_string.find("_class");
  if (pos != _doc_string.end())
    return pos->second;
  else
    return std::string();
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

  _doc_string = rhs._doc_string;
  _custom_type = rhs._custom_type;
  _group = rhs._group;
  _range_functions = rhs._range_functions;
  _auto_build_vectors = rhs._auto_build_vectors;
  _buildable_types = rhs._buildable_types;
  _collapse_nesting = rhs._collapse_nesting;
  _moose_object_syntax_visibility = rhs._moose_object_syntax_visibility;
  _required_params = rhs._required_params;
  _private_params = rhs._private_params;
  _valid_params = rhs._valid_params;
  _coupled_vars = rhs._coupled_vars;
  _syntax = rhs._syntax;
  _default_coupled_value = rhs._default_coupled_value;
  _default_postprocessor_value = rhs._default_postprocessor_value;
  _set_by_add_param = rhs._set_by_add_param;
  _allow_copy = rhs._allow_copy;
  _controllable_params = rhs._controllable_params;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters & rhs)
{
  Parameters::operator+=(rhs);

  _doc_string.insert(rhs._doc_string.begin(), rhs._doc_string.end());
  _custom_type.insert(rhs._custom_type.begin(), rhs._custom_type.end());
  _group.insert(rhs._group.begin(), rhs._group.end());
  _range_functions.insert(rhs._range_functions.begin(), rhs._range_functions.end());
  _auto_build_vectors.insert(rhs._auto_build_vectors.begin(), rhs._auto_build_vectors.end());
  _buildable_types.insert(
      _buildable_types.end(), rhs._buildable_types.begin(), rhs._buildable_types.end());
  // Collapse nesting and moose object syntax hiding are not modified with +=
  _required_params.insert(rhs._required_params.begin(), rhs._required_params.end());
  _private_params.insert(rhs._private_params.begin(), rhs._private_params.end());
  _valid_params.insert(rhs._valid_params.begin(), rhs._valid_params.end());
  _coupled_vars.insert(rhs._coupled_vars.begin(), rhs._coupled_vars.end());
  _syntax.insert(rhs._syntax.begin(), rhs._syntax.end());
  _default_coupled_value.insert(rhs._default_coupled_value.begin(),
                                rhs._default_coupled_value.end());
  _default_postprocessor_value.insert(rhs._default_postprocessor_value.begin(),
                                      rhs._default_postprocessor_value.end());
  _set_by_add_param.insert(rhs._set_by_add_param.begin(), rhs._set_by_add_param.end());
  _controllable_params.insert(rhs._controllable_params.begin(), rhs._controllable_params.end());
  return *this;
}

void
InputParameters::addCoupledVar(const std::string & name, Real value, const std::string & doc_string)
{
  // std::vector<VariableName>(1, Moose::stringify(value)),
  addParam<std::vector<VariableName>>(name, doc_string);
  _coupled_vars.insert(name);
  _default_coupled_value[name] = value;
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
  _auto_build_vectors[name] = std::make_pair(base_name, num_name);

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
  std::map<std::string, std::string>::const_iterator doc_string_it = _doc_string.find(name);
  if (doc_string_it != _doc_string.end())
    for (const auto & ch : doc_string_it->second)
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
  std::map<std::string, std::string>::iterator doc_string_it = _doc_string.find(name);
  if (doc_string_it == _doc_string.end())
    mooseError("Unable to set the documentation string (using setDocString) for the \"",
               name,
               "\" parameter, the parameter does not exist.");
  else
    doc_string_it->second = doc;
}

bool
InputParameters::isParamRequired(const std::string & name) const
{
  return _required_params.find(name) != _required_params.end();
}

bool
InputParameters::isParamValid(const std::string & name) const
{
  if (have_parameter<MooseEnum>(name))
    return get<MooseEnum>(name).isValid();
  else if (have_parameter<MultiMooseEnum>(name))
    return get<MultiMooseEnum>(name).isValid();
  else
    return _valid_params.find(name) != _valid_params.end();
}

bool
InputParameters::isParamSetByAddParam(const std::string & name) const
{
  return _set_by_add_param.find(name) != _set_by_add_param.end();
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
  return _private_params.find(name) != _private_params.end();
}

void
InputParameters::declareControllable(const std::string & input_names)
{
  std::vector<std::string> names;
  MooseUtils::tokenize<std::string>(input_names, names, 1, " ");
  _controllable_params.insert(names.begin(), names.end());
}

bool
InputParameters::isControllable(const std::string & name)
{
  return _controllable_params.find(name) != _controllable_params.end();
}

void
InputParameters::registerBase(const std::string & value)
{
  InputParameters::set<std::string>("_moose_base") = value;
  _private_params.insert("_moose_base");
}

void
InputParameters::registerBuildableTypes(const std::string & names)
{
  _buildable_types.clear();
  MooseUtils::tokenize(names, _buildable_types, 1, " \t\n\v\f\r"); // tokenize on whitespace
}

const std::vector<std::string> &
InputParameters::getBuildableTypes() const
{
  return _buildable_types;
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
  mooseError("Parameter '",                                                                        \
             name,                                                                                 \
             "' cannot be marked as controllable because its type (",                              \
             this->type(name),                                                                     \
             ") is not controllable.")

void
InputParameters::checkParams(const std::string & parsing_syntax)
{
  std::string l_prefix = this->have_parameter<std::string>("_object_name")
                             ? this->get<std::string>("_object_name")
                             : parsing_syntax;

  std::ostringstream oss;
  // Required parameters
  for (const auto & it : *this)
  {
    if (!isParamValid(it.first) && isParamRequired(it.first))
    {
      // The parameter is required but missing
      if (oss.str().empty())
        oss << "The following required parameters are missing:" << std::endl;
      oss << l_prefix << "/" << it.first << std::endl;
      oss << "\tDoc String: \"" + getDocString(it.first) + "\"" << std::endl;
    }
  }

  // Range checked parameters
  for (const auto & it : *this)
  {
    std::string long_name(l_prefix + "/" + it.first);

    dynamicCastRangeCheck(Real, Real, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(int, long, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(long, long, long_name, it.first, it.second, oss);
    dynamicCastRangeCheck(unsigned int, long, long_name, it.first, it.second, oss);
  }

  if (!oss.str().empty())
    mooseError(oss.str());

  // Controllable parameters
  for (const auto & param_name : _controllable_params)
  {
    // Check that parameter is valid
    if (!isParamValid(param_name))
      mooseError("The parameter '",
                 param_name,
                 "' is not a valid parameter for the object ",
                 l_prefix,
                 " thus cannot be marked as controllable.");

    if (isPrivate(param_name))
      mooseError("The parameter, '",
                 param_name,
                 "', in ",
                 l_prefix,
                 " is a private parameter and cannot be marked as controllable");

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
}

bool
InputParameters::hasCoupledValue(const std::string & coupling_name) const
{
  return _coupled_vars.find(coupling_name) != _coupled_vars.end();
}

bool
InputParameters::hasDefaultCoupledValue(const std::string & coupling_name) const
{
  return _default_coupled_value.find(coupling_name) != _default_coupled_value.end();
}

void
InputParameters::defaultCoupledValue(const std::string & coupling_name, Real value)
{
  _default_coupled_value[coupling_name] = value;
}

Real
InputParameters::defaultCoupledValue(const std::string & coupling_name) const
{
  std::map<std::string, Real>::const_iterator value_it = _default_coupled_value.find(coupling_name);

  if (value_it == _default_coupled_value.end())
    mooseError("Attempted to retrieve default value for coupled variable '",
               coupling_name,
               "' when none was provided. \n\nThere are three reasons why this may have "
               "occurred:\n 1. The other version of params.addCoupledVar() should be used in order "
               "to provide a default value. \n 2. This should have been a required coupled "
               "variable added with params.addRequiredCoupledVar() \n 3. The call to get the "
               "coupled value should have been properly guarded with isCoupled()\n");

  return value_it->second;
}

const std::map<std::string, std::pair<std::string, std::string>> &
InputParameters::getAutoBuildVectors() const
{
  return _auto_build_vectors;
}

std::string
InputParameters::type(const std::string & name)
{
  if (_coupled_vars.find(name) != _coupled_vars.end())
    return "std::vector<VariableName>";
  else if (_custom_type.find(name) != _custom_type.end())
    return _custom_type[name];
  else
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
    if (param_names.find(param_name) != param_names.end())
      _group[param_name] = group_name;
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
  return _syntax[name];
}

std::string
InputParameters::getGroupName(const std::string & param_name) const
{
  std::map<std::string, std::string>::const_iterator it = _group.find(param_name);

  if (it != _group.end())
    return it->second;
  else
    return std::string();
}

const PostprocessorValue &
InputParameters::getDefaultPostprocessorValue(const std::string & name, bool suppress_error) const
{
  // Check that a default exists, error if it does not
  std::map<std::string, PostprocessorValue>::const_iterator it =
      _default_postprocessor_value.find(name);
  if (!(suppress_error || it != _default_postprocessor_value.end()))
    mooseError("A default PostprcessorValue does not exist for the given name: ", name);

  // Return the value
  return it->second;
}

void
InputParameters::setDefaultPostprocessorValue(const std::string & name,
                                              const PostprocessorValue & value)
{
  _default_postprocessor_value[name] = value;
}

bool
InputParameters::hasDefaultPostprocessorValue(const std::string & name) const
{
  return _default_postprocessor_value.find(name) != _default_postprocessor_value.end();
}

void
InputParameters::applyParameters(const InputParameters & common,
                                 const std::vector<std::string> exclude)
{
  // Disable the display of deprecated message when applying common parameters, this avoids a dump
  // of messages
  _show_deprecated_message = false;

  // Loop through the common parameters
  for (const auto & it : common)
  {
    // Common parameter name
    const std::string & common_name = it.first;

    // Continue to next parameter, if the current is in list of  excluded parameters
    if (std::find(exclude.begin(), exclude.end(), common_name) != exclude.end())
      continue;

    // Extract the properties from the local parameter for the current common parameter name
    bool local_exist = _values.find(common_name) != _values.end();
    bool local_set = _set_by_add_param.find(common_name) == _set_by_add_param.end();
    bool local_priv = isPrivate(common_name);
    bool local_valid = isParamValid(common_name);

    // Extract the properties from the common parameter
    bool common_valid = common.isParamValid(common_name);
    bool common_priv = common.isPrivate(common_name);

    /* In order to apply common parameter 4 statements must be satisfied
     * (1) A local parameter must exist with the same name as common parameter
     * (2) Common parameter must valid
     * (3) Local parameter must be invalid OR not have been set from its default
     * (4) Neither may be private
     */
    if (local_exist && common_valid && (!local_valid || !local_set) &&
        (!common_priv || !local_priv))
    {
      delete _values[common_name];
      _values[common_name] = it.second->clone();
      set_attributes(common_name, false);
    }
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

    // If the local parameters has a coupled variable, populate it with the value from the common
    // parameters
    if (hasCoupledValue(var_name))
    {
      if (common.hasDefaultCoupledValue(var_name))
        addCoupledVar(
            var_name, common.defaultCoupledValue(var_name), common.getDocString(var_name));
      else
        addCoupledVar(var_name, common.getDocString(var_name));
    }
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
    return _set_by_add_param.find(name) == _set_by_add_param.end();
}

const std::string &
InputParameters::getDescription(const std::string & name)
{
  if (_doc_string.find(name) == _doc_string.end())
    mooseError("No parameter exists with the name ", name);
  return _doc_string[name];
}

template <>
void
InputParameters::addRequiredParam<MooseEnum>(const std::string & name,
                                             const MooseEnum & moose_enum,
                                             const std::string & doc_string)
{
  InputParameters::set<MooseEnum>(name) = moose_enum; // valid parameter is set by set_attributes
  _required_params.insert(name);
  _doc_string[name] = doc_string;
}

template <>
void
InputParameters::addRequiredParam<MultiMooseEnum>(const std::string & name,
                                                  const MultiMooseEnum & moose_enum,
                                                  const std::string & doc_string)
{
  InputParameters::set<MultiMooseEnum>(name) =
      moose_enum; // valid parameter is set by set_attributes
  _required_params.insert(name);
  _doc_string[name] = doc_string;
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
  _required_params.insert(name);
  _doc_string[name] = doc_string;
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
  _default_postprocessor_value[name] = r_value;

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
  _default_postprocessor_value[name] = r_value;

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
