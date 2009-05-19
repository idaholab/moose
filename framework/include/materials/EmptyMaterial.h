#include "Material.h"

#ifndef EMPTYMATERIAL_H
#define EMPTYMATERIAL_H

//Forward Declarations
class EmptyMaterial;

template<>
Parameters valid_params<EmptyMaterial>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class EmptyMaterial : public Material
{
public:
  EmptyMaterial(std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as)
  {}

protected:
  virtual void computeProperties();
};

#endif //EMPTYMATERIAL_H
