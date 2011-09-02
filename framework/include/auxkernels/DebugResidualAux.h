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

#ifndef DEBUGRESIDUALAUX_H
#define DEBUGRESIDUALAUX_H

#include "AuxKernel.h"

class DebugResidualAux;

template<>
InputParameters validParams<DebugResidualAux>();

/**
 * Auxiliary kernel for debugging convergence.
 */
class DebugResidualAux : public AuxKernel
{
public:
  DebugResidualAux(const std::string & name, InputParameters parameters);
  virtual ~DebugResidualAux();

protected:
  virtual Real computeValue();

  MooseVariable & _debug_var;
  NumericVector<Number> & _residual_copy;
};

#endif /* DEBUGRESIDUALAUX_H */
