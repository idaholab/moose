/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BNDSCALCAUX_H
#define BNDSCALCAUX_H

#include "AuxKernel.h"

//Forward Declarations
class BndsCalcAux;

template<>
InputParameters validParams<BndsCalcAux>();

class BndsCalcAux : public AuxKernel
{
public:

  BndsCalcAux(const InputParameters & parameters);
  BndsCalcAux(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual Real computeValue();

  std::vector<VariableValue *> _vals;
  unsigned int _ncrys;
};

#endif //BNDSCALCAUX_H
