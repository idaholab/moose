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
#ifndef FUNCTIONVALUEPOSTPROCESSOR_H
#define FUNCTIONVALUEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class FunctionValuePostprocessor;
class Function;

template <>
InputParameters validParams<FunctionValuePostprocessor>();

/**
 * This postprocessor displays a single value which is supplied by a MooseFunction.
 * It is designed to prevent the need to create additional
 * postprocessors like DifferencePostprocessor.
 */
class FunctionValuePostprocessor : public GeneralPostprocessor
{
public:
  FunctionValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  Function & _function;
  const Point & _point;
  const Real _scale_factor;
};

#endif /* FUNCTIONVALUEPOSTPROCESSOR_H */
