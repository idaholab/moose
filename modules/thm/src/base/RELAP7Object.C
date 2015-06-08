#include "RELAP7Object.h"

template<>
InputParameters validParams<RELAP7Object>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

RELAP7Object::RELAP7Object(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters)
{
}

RELAP7Object::~RELAP7Object()
{
}
