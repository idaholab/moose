#ifndef MTMATERIAL_H
#define MTMATERIAL_H

#include "Material.h"


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
  MTMaterial(const std::string & name,
             MooseSystem & moose_system,
             InputParameters parameters);
  
protected:
  virtual void computeProperties();
  
private:
  MaterialProperty<Real> & _mat_prop;
};

#endif //DIFF1MATERIAL_H
