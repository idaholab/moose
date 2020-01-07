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
 * Test and proof of concept class for computing UserObject Jacobians using the
 * ShapeElementUserObject base class. This object computes the integral
 * \f$ \int_\Omega u^2v dr \f$
 * and builds a vector of all derivatives of the integral w.r.t. the DOFs of u and v.
 * These Jacobian terms can be utilized by a Kernel that uses the integral in the
 * calculation of its residual.
 */
class ExampleShapeElementUserObject : public ShapeElementUserObject
{
public:
  static InputParameters validParams();

  ExampleShapeElementUserObject(const InputParameters & parameters);

  virtual ~ExampleShapeElementUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void executeJacobian(unsigned int jvar);
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  ///@{ custom UserObject interface functions
  const Real & getIntegral() const { return _integral; }
  const std::vector<Real> & getJacobian() const { return _jacobian_storage; }
  ///@}

protected:
  Real _integral;
  std::vector<Real> _jacobian_storage;

  const VariableValue & _u_value;
  unsigned int _u_var;
  const VariableValue & _v_value;
  unsigned int _v_var;
};
