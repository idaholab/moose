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

InputParameters emptyInputParameters()
{
  InputParameters params;
  return params;
}

InputParameters::InputParameters() :
    Parameters(),
    _collapse_nesting(false)
{
}

InputParameters::InputParameters(const InputParameters &rhs) :
    Parameters()
{
  *this = rhs;
}

InputParameters::InputParameters(const Parameters &rhs)
{
  Parameters::operator=(rhs);
  _collapse_nesting = false;
}

void
InputParameters::clear()
{
  Parameters::clear();
  _doc_string.clear();
  _custom_type.clear();
  _group.clear();
  _required_params.clear();
  _valid_params.clear();
  _private_params.clear();
  _coupled_vars.clear();
  _syntax.clear();
  _default_coupled_value.clear();
  _default_postprocessor_value.clear();
  _collapse_nesting = false;
}

void
InputParameters::addClassDescription(const std::string &doc_string)
{
  _doc_string["_class"] = doc_string;
}

void
InputParameters::set_attributes(const std::string & name, bool inserted_only)
{
  // valid_params don't make sense for MooseEnums
  if (!inserted_only && !have_parameter<MooseEnum>(name))
    _valid_params.insert(name);
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
InputParameters::operator=(const InputParameters &rhs)
{
  Parameters::operator=(rhs);

  this->_doc_string = rhs._doc_string;
  this->_custom_type = rhs._custom_type;
  this->_group = rhs._group;
  this->_buildable_types = rhs._buildable_types;
  this->_required_params = rhs._required_params;
  this->_private_params = rhs._private_params;
  this->_valid_params = rhs._valid_params;
  this->_coupled_vars = rhs._coupled_vars;
  this->_syntax = rhs._syntax;
  this->_default_coupled_value = rhs._default_coupled_value;
  this->_default_postprocessor_value = rhs._default_postprocessor_value;
  _collapse_nesting = rhs._collapse_nesting;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters &rhs)
{
  Parameters::operator+=(rhs);

  _doc_string.insert(rhs._doc_string.begin(), rhs._doc_string.end());
  _custom_type.insert(rhs._custom_type.begin(), rhs._custom_type.end());
  _group.insert(rhs._group.begin(), rhs._group.end());
  _buildable_types.insert(_buildable_types.end(), rhs._buildable_types.begin(), rhs._buildable_types.end());
  _required_params.insert(rhs._required_params.begin(), rhs._required_params.end());
  _private_params.insert(rhs._private_params.begin(), rhs._private_params.end());
  _valid_params.insert(rhs._valid_params.begin(), rhs._valid_params.end());
  _coupled_vars.insert(rhs._coupled_vars.begin(), rhs._coupled_vars.end());
  _syntax.insert(rhs._syntax.begin(), rhs._syntax.end());
  _default_coupled_value.insert(rhs._default_coupled_value.begin(), rhs._default_coupled_value.end());
  _default_postprocessor_value.insert(rhs._default_postprocessor_value.begin(), rhs._default_postprocessor_value.end());
  // Collapse nesting is not modified with +=

  return *this;
}

void
InputParameters::addCoupledVar(const std::string &name, Real value, const std::string &doc_string)
{
  _default_coupled_value[name] = value;

  addCoupledVar(name, doc_string);
}

void
InputParameters::addCoupledVar(const std::string &name, const std::string &doc_string)
{
  InputParameters::set<std::vector<VariableName> >(name);
  _doc_string[name] = doc_string;
  _coupled_vars.insert(name);
}

void
InputParameters::addRequiredCoupledVar(const std::string &name, const std::string &doc_string)
{
  InputParameters::insert<std::vector<VariableName> >(name);
  _required_params.insert(name);
  _doc_string[name] = doc_string;
  _coupled_vars.insert(name);
}

std::string
InputParameters::getDocString(const std::string &name) const
{
  std::string doc_string;
  std::map<std::string, std::string>::const_iterator doc_string_it = _doc_string.find(name);
  if (doc_string_it != _doc_string.end())
    for (std::string::const_iterator it = (doc_string_it->second).begin();
         it != (doc_string_it->second).end(); ++it)
    {
      if (*it == '\n')
        doc_string += " ... ";
      else
        doc_string += *it;
    }

  return doc_string;
}

bool
InputParameters::isParamRequired(const std::string &name) const
{
  return _required_params.find(name) != _required_params.end();
}

bool
InputParameters::isParamValid(const std::string &name) const
{
  if (have_parameter<MooseEnum>(name))
    return get<MooseEnum>(name).isValid();
  else
    return _valid_params.find(name) != _valid_params.end();
}

bool
InputParameters::areAllRequiredParamsValid() const
{
  for (Parameters::const_iterator it = this->begin(); it != this->end(); ++it)
    if (isParamRequired(it->first) && !isParamValid(it->first))
      return false;
  return true;
}

bool
InputParameters::isPrivate(const std::string &name) const
{
  return _private_params.find(name) != _private_params.end();
}

void
InputParameters::registerBase(const std::string &value)
{
  InputParameters::set<std::string>("_moose_base") = value;
  _private_params.insert("_moose_base");
}

void
InputParameters::registerBuildableTypes(const std::string &names)
{
  _buildable_types.clear();
  MooseUtils::tokenize(names, _buildable_types, 1, " \t\n\v\f\r");  // tokenize on whitespace
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
InputParameters::checkParams(const std::string &prefix) const
{
  std::string l_prefix = this->have_parameter<std::string>("long_name") ? this->get<std::string>("long_name") : prefix;

  for (InputParameters::const_iterator it = this->begin(); it != this->end(); ++it)
  {
    if (!isParamValid(it->first) && isParamRequired(it->first))
    {
      // The parameter is required but missing
      std::string doc = getDocString(it->first);
      mooseError("The required parameter '" + l_prefix + "/" + it->first + "' is missing\nDoc String: \"" +
                 getDocString(it->first) + "\"");
    }
  }
}

Real
InputParameters::defaultCoupledValue(std::string coupling_name)
{
  std::map<std::string, Real>::iterator value_it = _default_coupled_value.find(coupling_name);

  if(value_it == _default_coupled_value.end())
    mooseError("Attempted to retrieve default value for coupled variable '" << coupling_name << "' when none was provided. \n\nThere are three reasons why this may have occurred:\n 1. The other version of params.addCoupledVar() should be used in order to provde a default value. \n 2. This should have been a required coupled variable added with params.addRequiredCoupledVar() \n 3. The call to get the coupled value should have been properly guarded with isCoupled()\n");

  return value_it->second;
}

std::string
InputParameters::type(const std::string &name)
{
  if (_coupled_vars.find(name) != _coupled_vars.end())
    return "std::vector<VariableName>";
  else if (_custom_type.find(name) != _custom_type.end())
    return _custom_type[name];
  else
    return _values[name]->type();
}

std::string
InputParameters::getMooseType(const std::string &name) const
{
  std::string var;

  if (have_parameter<VariableName>(name))
    var = get<VariableName>(name);
  else if (have_parameter<NonlinearVariableName>(name))
    var = get<NonlinearVariableName>(name);
  else if (have_parameter<AuxVariableName>(name))
    var = get<AuxVariableName>(name);
  else if (have_parameter<std::string>(name))
    var = get<std::string>(name);

  return var;
}

std::vector<std::string>
InputParameters::getVecMooseType(const std::string &name) const
{
  std::vector<std::string> svars;

  if (have_parameter<std::vector<VariableName> >(name))
  {
    std::vector<VariableName> vars = get<std::vector<VariableName> >(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<NonlinearVariableName> >(name))
  {
    std::vector<NonlinearVariableName> vars = get<std::vector<NonlinearVariableName> >(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<AuxVariableName> >(name))
  {
    std::vector<AuxVariableName> vars = get<std::vector<AuxVariableName> >(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }
  else if (have_parameter<std::vector<std::string> >(name))
  {
    std::vector<std::string> vars = get<std::vector<std::string> >(name);
    std::copy(vars.begin(), vars.end(), std::back_inserter(svars));
  }

  return svars;
}

void
InputParameters::addParamNamesToGroup(const std::string &space_delim_names, const std::string group_name)
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(space_delim_names, elements, 1, " \t\n\v\f\r");  // tokenize on whitespace

  for (std::vector<std::string>::const_iterator it = elements.begin(); it != elements.end(); ++it)
    _group[*it] = group_name;

}


std::vector<std::string>
InputParameters::getSyntax(const std::string &name)
{
  return _syntax[name];
}

std::string
InputParameters::getGroupName(const std::string &param_name) const
{
  std::map<std::string, std::string>::const_iterator it = _group.find(param_name);

  if (it != _group.end())
    return it->second;
  else
    return std::string();
}

void
InputParameters::addPostprocessor(const std::string & name, Real default_value, const std::string &doc_string)
{
  // Add the postprocessor name parameter in traditional manner
  addParam<PostprocessorName>(name, doc_string);

  // Store the default value
  _default_postprocessor_value[name] = default_value;
}

PostprocessorValue &
InputParameters::defaultPostprocessorValue(const std::string & name)
{
  // Check that a default exists, error if it does not
  if (!hasDefaultPostprocessorValue(name))
    mooseError("A default PostprcessorValue does not exist for the given name: " << name);

  // Return the value
  return _default_postprocessor_value[name];
}

bool
InputParameters::hasDefaultPostprocessorValue(const std::string & name) const
{
  return _default_postprocessor_value.find(name) != _default_postprocessor_value.end();
}

void
InputParameters::applyParameters(const InputParameters & common)
{
  // Loop through the common parameters
  for(InputParameters::const_iterator it = common.begin(); it != common.end(); ++it)
  {
    // Extract value for setting criteria
    bool has = _values.find(it->first) != _values.end();
    bool valid = isParamValid(it->first);
    bool priv = isPrivate(it->first);

    // Apply the common parameter if it exists, is not set valid, and is not private
    if (has && !valid && !priv)
    {
      delete _values[it->first];
      _values[it->first] = it->second->clone();
      set_attributes(it->first, false);
    }
  }
}
