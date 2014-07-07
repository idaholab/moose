#ifndef FINITESTRAINPLASTICAUX_H
#define FINITESTRAINPLASTICAUX_H

#include "AuxKernel.h"
#include "FiniteStrainPlasticMaterial.h"

//Forward declarations
class FiniteStrainPlasticAux;

template<>
InputParameters validParams<FiniteStrainPlasticAux>();


/**
 * Obtains equivalent plastic strain for output
 */
class FiniteStrainPlasticAux : public AuxKernel
{
public:
  FiniteStrainPlasticAux(const std::string & name, InputParameters parameters);
  virtual ~FiniteStrainPlasticAux() {}

protected:
  virtual Real computeValue();

private:
  MaterialProperty<Real> & _eqv_plastic_strain;
};

#endif // FINITESTRAINPLASTICAUX_H
