//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHECKCURRENTEXECAUX_H
#define CHECKCURRENTEXECAUX_H

#include "AuxKernel.h"

class CheckCurrentExecAux;

template <>
InputParameters validParams<CheckCurrentExecAux>();

class CheckCurrentExecAux : public AuxKernel
{
public:
  CheckCurrentExecAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const FEProblemBase & _problem;
};

#endif // CHECKCURRENTEXECAUX_H
