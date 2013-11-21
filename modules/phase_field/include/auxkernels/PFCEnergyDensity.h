
#ifndef PFC_ENERGY_DENSITY_h
#define PFC_ENERGY_DENSITY_h

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
  Real computeValue();
  
  unsigned int _order;
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;
  std::vector<MaterialProperty<Real>* > _coeff;
  std::vector<VariableValue *> _vals;
  
};

#endif
