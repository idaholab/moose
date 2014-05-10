#ifndef PFC_ENERGY_DENSITY_H
#define PFC_ENERGY_DENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCEnergyDensity;

template<>
InputParameters validParams<PFCEnergyDensity>();

class PFCEnergyDensity : public AuxKernel
{
public:
   PFCEnergyDensity( const std::string& name, InputParameters parameters );

protected:
  virtual Real computeValue();

  std::vector<VariableValue *> _vals;
  std::vector<MaterialProperty<Real>* > _coeff;

  unsigned int _order;
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;
};

#endif //PFC_ENERGY_DENSITY_H
