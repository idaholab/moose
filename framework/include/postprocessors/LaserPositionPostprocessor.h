//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

class LaserPositionPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  LaserPositionPostprocessor(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {}
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:

  const PostprocessorValue & _speed;
  Real _current_arclength;
  Real _delta_arclength;
};
