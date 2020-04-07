#include "THMObject.h"

InputParameters
THMObject::validParams()
{
  InputParameters params = MooseObject::validParams();
  return params;
}

THMObject::THMObject(const InputParameters & parameters) : MooseObject(parameters) {}
