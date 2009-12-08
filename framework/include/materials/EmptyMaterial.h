#ifndef EMPTYMATERIAL_H
#define EMPTYMATERIAL_H

#include "Material.h"


//Forward Declarations
class EmptyMaterial;

template<>
InputParameters validParams<EmptyMaterial>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class EmptyMaterial : public Material
{
public:
  EmptyMaterial(std::string name,
           InputParameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
                std::vector<std::string> coupled_as);
  
protected:
  virtual void computeProperties();
};

#endif //EMPTYMATERIAL_H
