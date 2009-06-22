#include "EmptyMaterial.h"

template<>
Parameters valid_params<EmptyMaterial>()
{}

EmptyMaterial::EmptyMaterial(std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as)
  {}

void
EmptyMaterial::computeProperties()
{
}
