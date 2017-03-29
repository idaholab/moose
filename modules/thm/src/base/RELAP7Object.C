#include "RELAP7Object.h"

template <>
InputParameters
validParams<RELAP7Object>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

RELAP7Object::RELAP7Object(const InputParameters & parameters) : MooseObject(parameters) {}

RELAP7Object::~RELAP7Object() {}
