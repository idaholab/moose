#include "InputParameters.h"

InputParameters::InputParameters(const InputParameters &rhs)
{
  *this = rhs;
}

InputParameters::InputParameters(const Parameters &rhs)
{
  *this = rhs;
}

InputParameters &
InputParameters::operator=(const InputParameters &rhs)
{
  Parameters::operator=(rhs);

  this->_doc_string = rhs._doc_string;
  this->_required_params = rhs._required_params;

  return *this;
}

InputParameters &
InputParameters::operator+=(const InputParameters &rhs)
{
  Parameters::operator+=(rhs);
  
  std::map<std::string, std::string>::const_iterator doc_it = rhs._doc_string.begin();
  std::map<std::string, std::string>::const_iterator doc_end = rhs._doc_string.end();
  
  for(; doc_it != doc_end; ++doc_it)
    this->_doc_string[doc_it->first] = doc_it->second;

  std::set<std::string>::const_iterator req_it = rhs._required_params.begin();
  std::set<std::string>::const_iterator req_end = rhs._required_params.end();

  for(; req_it != req_end; ++req_it)
    this->_required_params.insert(*req_it);

  return *this;
}

void
InputParameters::seenInInputFile(const std::string &name)
{
  _valid_params.insert(name);
}
