#include "EmptyMaterial.h"

template<>
InputParameters validParams<EmptyMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}


EmptyMaterial::EmptyMaterial(const std::string & name, InputParameters parameters) :
  Material(name, parameters)
{
}

void
EmptyMaterial::computeProperties()
{
}
