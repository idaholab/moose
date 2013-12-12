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
  Real computeValue();

  int _p_num;
  int _dp_num;
  const RichardsDensity & _density_UO;
  const RichardsSeff & _seff_UO;
  const RichardsSat & _sat_UO;

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSMASSDENSITYPRIME_H
