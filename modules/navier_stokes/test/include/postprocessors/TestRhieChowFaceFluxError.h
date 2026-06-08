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
class RhieChowFaceFluxProvider;

class TestRhieChowFaceFluxError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestRhieChowFaceFluxError(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual PostprocessorValue getValue() const override;

protected:
  const RhieChowFaceFluxProvider & _rc_uo;
  const Function & _u;
  const Function & _v;
  const Function & _w;
  const MooseEnum _face_filter;
  PostprocessorValue _error;
};
