#ifndef COUPLEDMATERIAL_H_
#define COUPLEDMATERIAL_H_

#include "Material.h"


/**
 * A material that coupled a material property
 */
class CoupledMaterial : public Material
{
public:
  CoupledMaterial(const std::string & name,
                  InputParameters parameters);
  
protected:
  virtual void computeProperties();

  MaterialProperty<Real> & _my_prop;

  std::string _mat_prop_name;
  MaterialProperty<Real> & _coupled_mat_prop;
};

template<>
InputParameters validParams<CoupledMaterial>();

#endif //COUPLEDMATERIAL_H
