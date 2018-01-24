//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOTCOUPLINGAUX_H
#define DOTCOUPLINGAUX_H

#include "AuxKernel.h"

class DotCouplingAux;

template <>
InputParameters validParams<DotCouplingAux>();

/**
 * Couples in the time derivatives of a NL variable
 */
class DotCouplingAux : public AuxKernel
{
public:
  DotCouplingAux(const InputParameters & parameters);
  virtual ~DotCouplingAux();

protected:
  virtual Real computeValue();

  const VariableValue & _v_dot;
};

#endif /* DOTCOUPLINGAUX_H */
