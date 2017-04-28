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

#ifndef KERNEL_H
#define KERNEL_H

#include "KernelBase.h"

class Kernel;

template <>
InputParameters validParams<Kernel>();

class Kernel : public KernelBase
{
public:
  Kernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual Real computeQpResidual() = 0;

  /// Compute this Kernel's contribution to the Jacobian at the current quadrature point
  virtual Real computeQpJacobian();

  /// This is the virtual that derived classes should override for computing an off-diagonal Jacobian component.
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Following methods are used for Kernels that need to perform a per-element calculation
  virtual void precalculateResidual();
  virtual void precalculateJacobian() {}
  virtual void precalculateOffDiagJacobian(unsigned int /* jvar */) {}

  /// Reference to this Kernel's assembly object
  Assembly & _assembly;

  /// Reference to this Kernel's mesh object
  MooseMesh & _mesh;

  const Elem *& _current_elem;

  /// Volume of the current element
  const Real & _current_elem_volume;

  /// The current quadrature point index
  unsigned int _qp;

  /// The physical location of the element's quadrature Points, indexed by _qp
  const MooseArray<Point> & _q_point;

  /// active quadrature rule
  QBase *& _qrule;

  /// The current quadrature point weight value
  const MooseArray<Real> & _JxW;

  /// The scaling factor to convert from cartesian to another coordinate system (e.g rz, spherical, etc.)
  const MooseArray<Real> & _coord;

  /// current index for the test function
  unsigned int _i;

  /// current index for the shape function
  unsigned int _j;

  /// the current test function
  const VariableTestValue & _test;

  /// gradient of the test function
  const VariableTestGradient & _grad_test;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Time derivative of u
  const VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;
};

#endif /* KERNEL_H */
