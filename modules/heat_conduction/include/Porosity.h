#ifndef POROSITY_H
#define POROSITY_H

#include "Material.h"


//Forward Declarations
class Porosity;
class Function;

template<>
InputParameters validParams<Porosity>();

/**
 * Simple material with constant properties.
 */
class Porosity : public Material
{
public:
  Porosity(const std::string & name,
                         InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  const bool _has_temp;
  VariableValue & _temperature;

  const Real _my_porosity;

  MaterialProperty<Real> & _porosity;
  Function * const _porosity_temperature_function;
  const Real _anneal_temp;
//  MaterialProperty<Real> & _porosity_state;
//  MaterialProperty<Real> & _porosity_state_old;
  MaterialProperty<int> & _annealed;
  MaterialProperty<int> & _annealed_old;
};

#endif //POROSITY_H
