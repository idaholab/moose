#ifndef SILINEARISOTROPICMATERIAL_H
#define SILINEARISOTROPICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class SiLinearIsotropicMaterial;
class SymmElasticityTensor;

template<>
InputParameters validParams<SiLinearIsotropicMaterial>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class SiLinearIsotropicMaterial : public LinearIsotropicMaterial
{
public:
  SiLinearIsotropicMaterial(const std::string & name,
                          InputParameters parameters);
  
protected:
  virtual Real computeAlpha();
};

#endif //SILINEARISOTROPICMATERIAL_H
