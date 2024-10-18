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

/**
 * A postprocessor for testing e.g. that a parallel system vector has
 * not been inadvertently re-declared as ghosted
 */
class TestVectorType : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestVectorType(const InputParameters & parameters);

  virtual void initialize() override {}

  /**
   * Check that the specified vector is not ghosted
   */
  virtual void execute() override;

  /**
   * Return the summed value.
   */
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  /// The system to be tested
  SystemBase & _test_sys;

  /// The name of the vector to be tested
  std::string _test_vec_name;

  /// The type to expect to find
  libMesh::ParallelType _par_type;
};
