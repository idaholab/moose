//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "EqualValueEmbeddedConstraint.h"

/**
 * TestWriteableNodeElemConstraint tests writeable variables for NodeElemConstraints
 */
class TestWriteableNodeElemConstraint : public EqualValueEmbeddedConstraint
{
public:
  static InputParameters validParams();

  TestWriteableNodeElemConstraint(const InputParameters & parameters);

  virtual void reinitConstraint() override;

protected:
  // Output writeable Variable for testing; only writing to var1
  MooseWritableVariable * _output_writeable_var1 = nullptr;
  MooseWritableVariable * _output_writeable_var2 = nullptr;
};
