#ifndef RICHARDSSEFFAUX_H
#define RICHARDSSEFFAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

//Forward Declarations
class RichardsSeffAux;

template<>
InputParameters validParams<RichardsSeffAux>();

class RichardsSeffAux: public AuxKernel
{
public:
  RichardsSeffAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsSeff & _seff_UO;
  Real _p_air;
};

#endif // RICHARDSSEFFAUX_H
