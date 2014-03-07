#ifndef LANGMUIRMATERIAL_H
#define LANGMUIRMATERIAL_H

#include "Material.h"

//Forward Declarations
class LangmuirMaterial;

template<>
InputParameters validParams<LangmuirMaterial>();

class LangmuirMaterial : public Material
{
public:
  LangmuirMaterial(const std::string & name,
                  InputParameters parameters);

protected:

  virtual void computeQpProperties();


private:

  Real _mat_desorption_time_const;
  Real _mat_adsorption_time_const;
  VariableValue * _desorption_time_const_change;
  VariableValue * _adsorption_time_const_change;
  Real _mat_langmuir_density;
  Real _mat_langmuir_pressure;

  VariableValue * _pressure;

  MaterialProperty<Real> & _desorption_time_const;
  MaterialProperty<Real> & _adsorption_time_const;
  MaterialProperty<Real> & _equilib_conc;
  MaterialProperty<Real> & _equilib_conc_prime;

};

#endif //LANGMUIRMATERIAL_H
