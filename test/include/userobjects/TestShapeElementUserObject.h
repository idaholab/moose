//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ShapeElementUserObject.h"

/**
 * Internal test object for the ShapeElementUserObject class. This tests if the
 * _phi and _grad_phi get initialized to the correct sizes and that executeJacobian is called.
 * Do not use this as a starting point for developing your own ShapeElementUserObject
 * Use ExampleShapeElementUserObject instead!
 */
class TestShapeElementUserObject : public ShapeElementUserObject
{
public:
  static InputParameters validParams();

  TestShapeElementUserObject(const InputParameters & parameters);

  virtual ~TestShapeElementUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void executeJacobian(unsigned int jvar);
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  /// Bit mask for testing purposes (bits get set if executeJacobian is called for a variable)
  unsigned int _execute_mask;

  unsigned int _u_var;
  unsigned int _u_dofs;
  unsigned int _v_var;
  unsigned int _v_dofs;
};
