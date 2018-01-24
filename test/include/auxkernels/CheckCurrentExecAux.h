/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
