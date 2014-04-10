#ifndef VARCOUPLINGMATERIALEIGEN_H_
#define VARCOUPLINGMATERIALEIGEN_H_

#include "Material.h"

class VarCouplingMaterialEigen;

template<>
InputParameters validParams<VarCouplingMaterialEigen>();

/**
 * A material that couples a variable
 */
class VarCouplingMaterialEigen : public Material
{
public:
  VarCouplingMaterialEigen(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  VariableValue & _var;
  VariableValue & _var_old;
  std::string _propname;
  MaterialProperty<Real> & _mat;
  MaterialProperty<Real> & _mat_old;
};

#endif //VARCOUPLINGMATERIALEIGEN_H
