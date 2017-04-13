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
