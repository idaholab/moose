/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TESTSHAPEELEMENTUSEROBJECT_H
#define TESTSHAPEELEMENTUSEROBJECT_H

#include "ShapeElementUserObject.h"

// Forward Declarations
class TestShapeElementUserObject;

template <>
InputParameters validParams<TestShapeElementUserObject>();

/**
 * Internal test object for the ShapeElementUserObject class. This tests if the
 * _phi and _grad_phi get initialized to the correct sizes and that executeJacobian is called.
 * Do not use this as a starting point for developing your own ShapeElementUserObject
 * Use ExampleShapeElementUserObject instead!
 */
class TestShapeElementUserObject : public ShapeElementUserObject
{
public:
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

#endif
