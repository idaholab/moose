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

/**
 * Errors if the PETSc vectors or matrices of a system do not have the expected PETSc types
 */
class TestPetscObjectTypes : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestPetscObjectTypes(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

  using Postprocessor::getValue;
  virtual Real getValue() const override { return 1; }

protected:
  /// The system whose vectors and matrices are checked
  const SystemBase & _test_sys;
};
