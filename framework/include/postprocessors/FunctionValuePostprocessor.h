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
 * Displays a value as supplied by the function FunctionName
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
  //const PostprocessorValue & _value1;
  Function & _function;

};


#endif /* FUNCTIONVALUEPOSTPROCESSOR_H */
