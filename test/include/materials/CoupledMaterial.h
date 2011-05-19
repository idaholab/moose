#ifndef COUPLEDMATERIAL_H_
#define COUPLEDMATERIAL_H_

#include "Material.h"

class CoupledMaterial;

template<>
InputParameters validParams<CoupledMaterial>();

/**
 * A material that couples a material property
 */
class CoupledMaterial : public Material
{
public:
  CoupledMaterial(const std::string & name, InputParameters parameters);
  
protected:
  virtual void computeProperties();

  std::string _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  std::string _coupled_mat_prop_name;
  MaterialProperty<Real> & _coupled_mat_prop;
};

#endif //COUPLEDMATERIAL_H
