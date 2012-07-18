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
#include "Moose.h"
#include "MooseTypes.h"

InputParameters emptyInputParameters()
{
  InputParameters params;
  return params;
}

InputParameters::InputParameters(const InputParameters &rhs) : Parameters()
{
  *this = rhs;
}

InputParameters::InputParameters(const Parameters &rhs)
{
  Parameters::operator=(rhs);
}

void
InputParameters::clear()
{
  Parameters::clear();
  _doc_string.clear();
  _custom_type.clear();
  _required_params.clear();
  _valid_params.clear();
  _private_params.clear();
  _seen_in_input.clear();
  _coupled_vars.clear();
}

void
InputParameters::addClassDescription(const std::string &doc_string)
{
  _doc_string["_class"] = doc_string;
}

void
InputParameters::set_attributes(const std::string & name, bool inserted_only)
{
  if (!inserted_only)
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

/*
Parameters &
InputParameters::operator=(const Parameters &rhs)
{
  Parameters::operator=(rhs);
  return *this;
}

Parameters &
InputParameters::operator+=(const Parameters &rhs)
{
  Parameters::operator+=(rhs);
  return *this;
}
*/

InputParameters &
InputParameters::operator=(const InputParameters &rhs)
{
  Parameters::operator=(rhs);

  this->_doc_string = rhs._doc_string;
  this->_custom_type = rhs._custom_type;
  this->_required_params = rhs._required_params;
  this->_private_params = rhs._private_params;
  this->_valid_params = rhs._valid_params;
  this->_coupled_vars = rhs._coupled_vars;
  this->_seen_in_input = rhs._seen_in_input;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters &rhs)
{
  Parameters::operator+=(rhs);

  _doc_string.insert(rhs._doc_string.begin(), rhs._doc_string.end());
  _custom_type.insert(rhs._custom_type.begin(), rhs._custom_type.end());
  _required_params.insert(rhs._required_params.begin(), rhs._required_params.end());
  _private_params.insert(rhs._private_params.begin(), rhs._private_params.end());
  _valid_params.insert(rhs._valid_params.begin(), rhs._valid_params.end());
  _coupled_vars.insert(rhs._coupled_vars.begin(), rhs._coupled_vars.end());
  _seen_in_input.insert(rhs._seen_in_input.begin(), rhs._seen_in_input.end());

  return *this;
}

void
InputParameters::addCoupledVar(const std::string &name, const std::string &doc_string)
{
  Parameters::set<std::vector<std::string> >(name);
  _doc_string[name] = doc_string;
  _coupled_vars.insert(name);
}

void
InputParameters::addRequiredCoupledVar(const std::string &name, const std::string &doc_string)
{
  Parameters::set<std::vector<std::string> >(name);
  _required_params.insert(name);
  _doc_string[name] = doc_string;
  _coupled_vars.insert(name);
}

void
InputParameters::seenInInputFile(const std::string &name)
{
  _valid_params.insert(name);
  _seen_in_input.insert(name);
}

std::string
InputParameters::getDocString(const std::string &name) const
{
  std::string doc_string;
  std::map<std::string, std::string>::const_iterator doc_string_it = _doc_string.find(name);
  if (doc_string_it != _doc_string.end())
    for (std::string::const_iterator it = (doc_string_it->second).begin();
         it != (doc_string_it->second).end(); ++it)
      if (*it == '\n')
        doc_string += " ... ";
      else
        doc_string += *it;

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
  return _valid_params.find(name) != _valid_params.end();
}

bool
InputParameters::wasSeenInInput(const std::string &name) const
{
  return _seen_in_input.find(name) != _seen_in_input.end();
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
InputParameters::checkParams(const std::string &prefix) const
{
  for (InputParameters::const_iterator it = this->begin(); it != this->end(); ++it)
  {
    std::string orig_name = prefix + "/" + it->first;

    if (!isParamValid(it->first) && isParamRequired(it->first))
    {
      // The parameter is required but missing
      std::string doc = getDocString(it->first);
      mooseError("The required parameter '" + orig_name + "' is missing\nDoc String: \"" +
                 getDocString(it->first) + "\"");
    }
  }
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
