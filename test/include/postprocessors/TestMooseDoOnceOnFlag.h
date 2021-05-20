//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class TestMooseDoOnceOnFlag : public GeneralPostprocessor
{
public:
  TestMooseDoOnceOnFlag(const InputParameters & parameters);

  static InputParameters validParams();

  void execute() override final;
  Real getValue() override final;

  void initialize() final{};
  void finalize() final{};

private:
  Real _value;
};
