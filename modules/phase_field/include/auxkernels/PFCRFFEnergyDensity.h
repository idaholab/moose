
#ifndef PFCRFF_ENERGY_DENSITY_h
#define PFCRFF_ENERGY_DENSITY_h

#include "AuxKernel.h"
#include <sstream>

class PFCRFFEnergyDensity;

template<>
InputParameters validParams<PFCRFFEnergyDensity>();

class PFCRFFEnergyDensity : public AuxKernel
{
public:
   PFCRFFEnergyDensity( const std::string& name, InputParameters parameters );

protected:
  Real computeValue();
  
  unsigned int _order;
  Real _a;
  Real _b;
  Real _c;
  std::vector<VariableValue *> _vals;
  
};

#endif
