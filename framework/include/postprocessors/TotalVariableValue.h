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

#ifndef TOTALVARIABLEVALUE_H
#define TOTALVARIABLEVALUE_H

#include "GeneralPostprocessor.h"

class TotalVariableValue;

template<>
InputParameters validParams<TotalVariableValue>();

/**
 * Integrate a post-processor value over time using trapezoidal rule
 */
class TotalVariableValue : public GeneralPostprocessor
{
public:
  TotalVariableValue(const std::string & name, InputParameters parameters);
  virtual ~TotalVariableValue();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  /// The total value of the variable
  Real _value;

  /// My old value
  const PostprocessorValue & _value_old;

  /// The current post-processor value
  const PostprocessorValue & _pps_value;

  /// The old post-processor value
  const PostprocessorValue & _pps_value_old;
};

#endif /* TOTALVARIABLEVALUE_H */
