#include <vector>
#include "InputParameters.h"
#include "Moose.h"

InputParameters::InputParameters(const InputParameters &rhs) : Parameters()
{
  *this = rhs;
}

InputParameters::InputParameters(const Parameters &rhs)
{
  Parameters::operator=(rhs);
}

InputParameters &
InputParameters::operator=(const InputParameters &rhs)
{
  Parameters::operator=(rhs);

  this->_doc_string = rhs._doc_string;
  this->_required_params = rhs._required_params;
  this->_private_params = rhs._private_params;
  this->_valid_params = rhs._valid_params;
  this->_coupled_vars = rhs._coupled_vars;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters &rhs)
{
  Parameters::operator+=(rhs);

  for(std::map<std::string, std::string>::const_iterator it = rhs._doc_string.begin();
      it!=rhs._doc_string.end();
      ++it)
    this->_doc_string[it->first] = it->second;

  for(std::set<std::string>::const_iterator it = rhs._required_params.begin();
      it!=rhs._required_params.end();
      ++it)
    this->_required_params.insert(*it);

  for(std::set<std::string>::const_iterator it = rhs._private_params.begin();
      it!=rhs._private_params.end();
      ++it)
    this->_private_params.insert(*it);

  for(std::set<std::string>::const_iterator it = rhs._coupled_vars.begin();
      it!=rhs._coupled_vars.end();
      ++it)
    this->_coupled_vars.insert(*it);

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
}

const std::string &
InputParameters::getDocString(const std::string &name) const
{
  static std::string empty;
  std::map<std::string, std::string>::const_iterator doc_string = _doc_string.find(name);
  return doc_string != _doc_string.end() ? doc_string->second : empty;
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
InputParameters::isPrivate(const std::string &name) const
{
  return _private_params.find(name) != _private_params.end();
}

