//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIPLEUPDATEAUX_H_
#define MULTIPLEUPDATEAUX_H_

#include "AuxKernel.h"

class MultipleUpdateAux;

template <>
InputParameters validParams<MultipleUpdateAux>();

/**
 * Aux kernel that updated values of coupled variables
 */
class MultipleUpdateAux : public AuxKernel
{
public:
  MultipleUpdateAux(const InputParameters & parameters);
  virtual ~MultipleUpdateAux();

protected:
  virtual Real computeValue();

  const VariableValue & _nl_u;
  VariableValue & _var1;
  VariableValue & _var2;
};

#endif /* MULTIPLEUPDATEAUX_H_ */
