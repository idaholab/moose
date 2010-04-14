#include "EmptyMaterial.h"

template<>
InputParameters validParams<EmptyMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}

EmptyMaterial::EmptyMaterial(std::string name,
                             MooseSystem & moose_system,
                             InputParameters parameters)
  :Material(name, moose_system, parameters)
{}

void
EmptyMaterial::computeProperties()
{}
