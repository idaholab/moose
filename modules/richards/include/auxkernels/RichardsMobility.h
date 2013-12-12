#ifndef RICHARDSMOBILITY_H
#define RICHARDSMOBILITY_H

#include "AuxKernel.h"

#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsRelPerm.h"

//Forward Declarations
class RichardsMobility;

template<>
InputParameters validParams<RichardsMobility>();

class RichardsMobility: public AuxKernel
{
public:
  RichardsMobility(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsDensity & _density_UO;
  const RichardsSeff & _seff_UO;
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSMOBILITY_H
