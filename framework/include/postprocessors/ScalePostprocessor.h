//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALEPOSTPROCESSOR_H
#define SCALEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class ScalePostprocessor;

template <>
InputParameters validParams<ScalePostprocessor>();

/**
 * Scale a postprocessor
 */
class ScalePostprocessor : public GeneralPostprocessor
{
public:
  ScalePostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual PostprocessorValue getValue() override;

protected:
  const PostprocessorValue & _value;
  Real _scaling_factor;
};

#endif /* SCALEPOSTPROCESSOR_H */
