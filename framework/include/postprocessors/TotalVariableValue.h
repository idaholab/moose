//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TOTALVARIABLEVALUE_H
#define TOTALVARIABLEVALUE_H

#include "GeneralPostprocessor.h"

class TotalVariableValue;

template <>
InputParameters validParams<TotalVariableValue>();

/**
 * Integrate a post-processor value over time using trapezoidal rule
 */
class TotalVariableValue : public GeneralPostprocessor
{
public:
  TotalVariableValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

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
