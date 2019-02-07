#include "THMObject.h"

template <>
InputParameters
validParams<THMObject>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

THMObject::THMObject(const InputParameters & parameters) : MooseObject(parameters) {}
