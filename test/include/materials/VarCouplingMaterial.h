#ifndef VARCOUPLINGMATERIAL_H_
#define VARCOUPLINGMATERIAL_H_

#include "Material.h"

class VarCouplingMaterial;

template<>
InputParameters validParams<VarCouplingMaterial>();

/**
 * A material that couples a variable
 */
class VarCouplingMaterial : public Material
{
public:
  VarCouplingMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  VariableValue & _var;
  MaterialProperty<Real> & _diffusion;
};

#endif //VARCOUPLINGMATERIAL_H
