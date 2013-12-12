#ifndef RICHARDSMOBILITYPRIME_H
#define RICHARDSMOBILITYPRIME_H

#include "AuxKernel.h"

#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsRelPerm.h"

//Forward Declarations
class RichardsMobilityPrime;

template<>
InputParameters validParams<RichardsMobilityPrime>();

class RichardsMobilityPrime: public AuxKernel
{
public:
  RichardsMobilityPrime(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsDensity & _density_UO;
  const RichardsSeff & _seff_UO;
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSMOBILITYPRIME_H
