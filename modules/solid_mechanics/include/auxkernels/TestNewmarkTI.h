//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Stores the velocity/acceleration computed using the time integrator into
 * the provided auxvariable
 */
class TestNewmarkTI : public AuxKernel
{
public:
  static InputParameters validParams();

  TestNewmarkTI(const InputParameters & parameters);

  virtual ~TestNewmarkTI() {}

protected:
  virtual Real computeValue();

  /// Parameter that decides whether first or second derivative should be stored
  const bool _first;

  /// Value of the first/second time derivative of dispalcement
  const VariableValue & _value;
};
