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
InputParameters::operator=(const Parameters &rhs)
{
  Parameters::operator=(rhs);

  // Should we zero out the rest of the members?
  return *this;
}

