//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class Function;
class RhieChowMassFlux;

class TestRhieChowPressureGradientError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestRhieChowPressureGradientError(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual PostprocessorValue getValue() const override;

protected:
  const RhieChowMassFlux & _rc_uo;
  const Function & _grad_x;
  const Function & _grad_y;
  const Function & _grad_z;
  const MooseEnum _cell_filter;
  PostprocessorValue _error;
};
