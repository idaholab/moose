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

template<>
InputParameters validParams<FunctionValuePostprocessor>();

/**
 * This postprocessor displays a single value which is supplied by a MooseFunction.
 * It is designed to prevent the need to create additional
 * postprocessors like DifferencePostprocessor.
 */
class FunctionValuePostprocessor : public GeneralPostprocessor
{
public:
  FunctionValuePostprocessor(const std::string & name, InputParameters parameters);
  virtual ~FunctionValuePostprocessor();

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();
  virtual void threadJoin(const UserObject & uo);

protected:
  Function & _function;
};

#endif /* FUNCTIONVALUEPOSTPROCESSOR_H */
