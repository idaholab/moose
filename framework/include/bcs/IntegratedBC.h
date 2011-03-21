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

#ifndef INTEGRATEDBC_H
#define INTEGRATEDBC_H

#include "BoundaryCondition.h"
#include "Coupleable.h"
#include "MooseVariable.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class IntegratedBC :
  public BoundaryCondition,
  public Coupleable
{
public:
  IntegratedBC(const std::string & name, InputParameters parameters);
  virtual ~IntegratedBC();

  virtual void computeResidual();
  virtual void computeJacobian(int i, int j);
  /**
   * Computes d-ivar-residual / d-jvar... storing the result in Ke.
   */
  void computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar);

protected:
  MooseVariable & _test_var;

  unsigned int _qp;
  QBase * & _qrule;
  const std::vector< Point > & _q_point;
  const std::vector<Real> & _JxW;
  unsigned int _i, _j;

  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;
  const std::vector<std::vector<RealGradient> > & _grad_test;
  const std::vector<std::vector<RealTensor> > & _second_test;
  // unknown
  const VariableValue & _u;
  const VariableGradient & _grad_u;
  const VariableSecond & _second_u;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};

template<>
InputParameters validParams<IntegratedBC>();

#endif /* INTEGRATEDBC_H */
