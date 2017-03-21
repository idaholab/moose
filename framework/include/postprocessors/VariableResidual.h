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

#ifndef VARIABLERESIDUAL_H
#define VARIABLERESIDUAL_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class VariableResidual;

template <>
InputParameters validParams<VariableResidual>();

class VariableResidual : public GeneralPostprocessor
{
public:
  VariableResidual(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual PostprocessorValue getValue() override;

protected:
  /// MOOSE variable we compute the residual for
  MooseVariable & _var;
  /// The residual of the variable
  Real _var_residual;
};

#endif // VARIABLERESIDUAL_H
