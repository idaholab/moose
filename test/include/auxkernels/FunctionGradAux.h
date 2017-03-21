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

#ifndef FUNCTIONGRADAUX_H
#define FUNCTIONGRADAUX_H

#include "AuxKernel.h"

// Forward Declarations
class FunctionGradAux;
class Function;

template <>
InputParameters validParams<FunctionGradAux>();

/**
 * AuxKernel for computing the gradient of a function and selecting one component
 * to be saved in the AuxVariable
 */
class FunctionGradAux : public AuxKernel
{
public:
  FunctionGradAux(const InputParameters & parameters);

  virtual ~FunctionGradAux();

protected:
  virtual Real computeValue();

  /// Function object from which gradient is retrieved
  Function & _func;

  /// The dimension index: 0|1|2 for x|y|z
  unsigned int _dim_index;
};

#endif // FunctionGradAux_H
