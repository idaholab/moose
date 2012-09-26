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
#include "MooseVariableInterface.h"
#include "MooseVariable.h"
#include "MaterialPropertyInterface.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class IntegratedBC;

template<>
InputParameters validParams<IntegratedBC>();

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class IntegratedBC :
  public BoundaryCondition,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public MaterialPropertyInterface
{
public:
  IntegratedBC(const std::string & name, InputParameters parameters);

  virtual void computeResidual();
  virtual void computeJacobian();
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  void computeJacobianBlock(unsigned int jvar);

protected:
  /// current element
  const Elem * & _current_elem;
  /// current side of the current element
  unsigned int & _current_side;
  /// current side element
  const Elem * & _current_side_elem;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// quadrature point index
  unsigned int _qp;
  /// active quadrature rule
  QBase * & _qrule;
  /// active quadrature points
  const MooseArray< Point > & _q_point;
  /// transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  /// coordinate transformation
  const MooseArray<Real> & _coord;
  /// i-th, j-th index for enumerating test and shape functions
  unsigned int _i, _j;

  // shape functions

  /// shape function values (in QPs)
  const VariablePhiValue & _phi;
  /// gradients of shape functions (in QPs)
  const VariablePhiGradient & _grad_phi;

  // test functions

  /// test function values (in QPs)
  const VariableTestValue & _test;
  /// gradients of test functions  (in QPs)
  const VariableTestGradient & _grad_test;

  // unknown

  /// the values of the unknown variable this BC is acting on
  const VariableValue & _u;
  /// the gradient of the unknown variable this BC is acting on
  const VariableGradient & _grad_u;

  /// Holds residual entries as their accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable*> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};

#endif /* INTEGRATEDBC_H */
