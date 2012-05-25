#ifndef COUPLEDMATERIAL2_H_
#define COUPLEDMATERIAL2_H_

#include "Material.h"

class CoupledMaterial2;

template<>
InputParameters validParams<CoupledMaterial2>();

/**
 * A material that couples a material property
 */
class CoupledMaterial2 : public Material
{
public:
  CoupledMaterial2(const std::string & name, InputParameters parameters);

protected:
  virtual void computeProperties();

  std::string _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  std::string _coupled_mat_prop_name;
  MaterialProperty<Real> & _coupled_mat_prop;

  std::string _coupled_mat_prop_name2;
  MaterialProperty<Real> & _coupled_mat_prop2;
};

#endif //COUPLEDMATERIAL2_H
