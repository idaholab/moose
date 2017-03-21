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

#ifndef FUNCTIONSCALARAUX_H
#define FUNCTIONSCALARAUX_H

#include "AuxScalarKernel.h"

class FunctionScalarAux;
class Function;

template <>
InputParameters validParams<FunctionScalarAux>();

/**
 * Sets a value of a scalar variable based on the function
 */
class FunctionScalarAux : public AuxScalarKernel
{
public:
  FunctionScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  std::vector<Function *> _functions;
};

#endif /* FUNCTIONSCALARAUX_H */
