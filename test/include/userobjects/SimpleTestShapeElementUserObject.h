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

#ifndef SIMPLETESTSHAPEELEMENTUSEROBJECT_H
#define SIMPLETESTSHAPEELEMENTUSEROBJECT_H

#include "ShapeElementUserObject.h"

// Forward Declarations
class SimpleTestShapeElementUserObject;

template <>
InputParameters validParams<SimpleTestShapeElementUserObject>();

/**
 * Test and proof of concept class for computing UserObject Jacobians using the
 * ShapeElementUserObject base class. This object computes the integral
 * \f$ \int_\Omega u dr \f$
 * and builds a vector of all derivatives of the integral w.r.t. the DOFs of u and v.
 * These Jacobian terms can be utilized by a Kernel that uses the integral in the
 * calculation of its residual.
 */
class SimpleTestShapeElementUserObject : public ShapeElementUserObject
{
public:
  SimpleTestShapeElementUserObject(const InputParameters & parameters);

  virtual ~SimpleTestShapeElementUserObject() {}

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
};

#endif
