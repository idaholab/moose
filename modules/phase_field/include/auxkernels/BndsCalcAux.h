/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BndsCalcAux_H
#define BndsCalcAux_H

#include "AuxKernel.h"

//Forward Declarations
class BndsCalcAux;

template<>
InputParameters validParams<BndsCalcAux>();

class BndsCalcAux : public AuxKernel
{
public:

  BndsCalcAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  std::vector<VariableValue *> _vals;
  unsigned int _ncrys;
};

#endif //BndsCalcAux_H
