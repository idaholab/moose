//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DIFFERENCEPOSTPROCESSOR_H
#define DIFFERENCEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class DifferencePostprocessor;

template <>
InputParameters validParams<DifferencePostprocessor>();

/**
 * Computes the difference between two postprocessors
 *
 * result = value1 - value2
 */
class DifferencePostprocessor : public GeneralPostprocessor
{
public:
  DifferencePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  const PostprocessorValue & _value1;
  const PostprocessorValue & _value2;
};

#endif /* DIFFERENCEPOSTPROCESSOR_H */
