#ifndef DIFF2MATERIAL_H
#define DIFF2MATERIAL_H

#include "Material.h"

//Forward Declarations
class Diff2Material;

template<>
InputParameters validParams<Diff2Material>();

/**
 * Simple material with constant properties.
 */
class Diff2Material : public Material
{
public:
  Diff2Material(std::string name,
                MooseSystem & moose_system,
                InputParameters parameters);
  
protected:
  virtual void computeProperties();
  
private:
  Real _diff;         // the value read from the input file
  MaterialProperty<Real> & _diffusivity;
};

#endif //DIFF2MATERIAL_H
