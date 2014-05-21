#ifndef STOCHASTICFIELDAUX_H
#define STOCHASTICFIELDAUX_H

#include "AuxKernel.h"
#include "StochasticField.h"

class StochasticFieldAux;

template<>
InputParameters validParams<StochasticFieldAux>();

class StochasticFieldAux : public AuxKernel
{
public:
  StochasticFieldAux(const std::string & name, InputParameters parameters);
  virtual ~StochasticFieldAux() {};

protected:
  virtual Real computeValue();

private:
  StochasticField  _stoch_field;
};

#endif // STOCHASTICFIELDAUX_H
