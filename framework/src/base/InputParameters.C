#include "InputParameters.h"

InputParameters::InputParameters(const InputParameters &rhs)
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

  return *this;
}

void
InputParameters::seenInInputFile(const std::string &name)
{
  _valid_params.insert(name);
}
