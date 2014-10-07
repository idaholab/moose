#ifndef VECRANGECHECKMATERIAL_H
#define VECRANGECHECKMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"


//Forward Declarations
class VecRangeCheckMaterial;

template<>
InputParameters validParams<VecRangeCheckMaterial>();

/**
 * Simple material to test vector parameter range checking.
 */
class VecRangeCheckMaterial : public Material
{
public:
  VecRangeCheckMaterial(const std::string & name, InputParameters parameters);
};

#endif //VECRANGECHECKMATERIAL_H
