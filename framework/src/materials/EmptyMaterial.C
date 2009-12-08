#include "EmptyMaterial.h"

template<>
InputParameters validParams<EmptyMaterial>()
{
  InputParameters params;
  return params;
}

EmptyMaterial::EmptyMaterial(std::string name,
           InputParameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as)
  {}

void
EmptyMaterial::computeProperties()
{
}
