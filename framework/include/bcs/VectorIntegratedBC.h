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

#ifndef VECTORINTEGRATEDBC_H
#define VECTORINTEGRATEDBC_H

#include "IntegratedBCBase.h"
#include "MooseVariableInterface.h"

// Forward declarations
class VectorIntegratedBC;

template <>
InputParameters validParams<VectorIntegratedBC>();

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class VectorIntegratedBC : public IntegratedBCBase, public MooseVariableInterface<RealVectorValue>
{
public:
  VectorIntegratedBC(const InputParameters & parameters);

  virtual VectorMooseVariable & variable() override { return _var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  virtual void computeJacobianBlock(MooseVariableFEBase & jvar) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  void computeJacobianBlockScalar(unsigned int jvar) override;

protected:
  /**
   * method for computing the residual at quadrature points
   */
  virtual Real computeQpResidual() = 0;

  VectorMooseVariable & _var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  // shape functions

  /// shape function values (in QPs)
  const VectorVariablePhiValue & _phi;

  // test functions

  /// test function values (in QPs)
  const VectorVariableTestValue & _test;

  // solution variable

  /// the values of the unknown variable this BC is acting on
  const VectorVariableValue & _u;
};

#endif /* VECTORINTEGRATEDBC_H */
