#ifndef RICHARDSMASSDENSITYPRIME_H
#define RICHARDSMASSDENSITYPRIME_H

#include "AuxKernel.h"

#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"

//Forward Declarations
class RichardsMassDensityPrime;

template<>
InputParameters validParams<RichardsMassDensityPrime>();

class RichardsMassDensityPrime: public AuxKernel
{
public:
  RichardsMassDensityPrime(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsDensity & _density_UO;
  const RichardsSeff & _seff_UO;
  const RichardsSat & _sat_UO;
  Real _p_air;
};

#endif // RICHARDSMASSDENSITYPRIME_H
