#ifndef MTMATERIAL_H
#define MTMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"


//Forward Declarations
class MTMaterial;

template<>
InputParameters validParams<MTMaterial>();

/**
 * Simple material with constant properties.
 */
class MTMaterial : public Material
{
public:
  MTMaterial(const std::string & name, InputParameters parameters);
  
protected:
  virtual void computeQpProperties();
  
  MaterialProperty<Real> & _mat_prop;
  Real _value;
};

#endif //DIFF1MATERIAL_H
