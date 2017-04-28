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

#ifndef EIGENKERNEL_H
#define EIGENKERNEL_H

#include "KernelBase.h"

// Forward Declarations
class EigenKernel;
class MooseEigenSystem;

template <>
InputParameters validParams<EigenKernel>();

/**
 * The behavior of this kernel is controlled by one problem-wise global parameter
 *    eigen_on_current - bool, to indicate if this kernel is operating on the current solution or
 * old solution
 * This kernel also obtain the postprocessor for eigenvalue by one problem-wise global parameter
 *    eigen_postprocessor - string, the name of the postprocessor to obtain the eigenvalue
 */
class EigenKernel : public KernelBase
{
public:
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/) override;
  virtual void computeOffDiagJacobianScalar(unsigned int /*jvar*/) override {}

  EigenKernel(const InputParameters & parameters);
  virtual bool enabled() override;

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian() { return 0; }
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }

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

  /// flag for as an eigen kernel or a normal kernel
  bool _eigen;

  /// EigenKernel always lives in EigenSystem
  MooseEigenSystem * _eigen_sys;

  /**
   * A pointer to the eigenvalue that is stored in a postprocessor
   * This is a pointer so that the method for retrieval (old vs current) may be changed.
   */
  const Real * _eigenvalue;
};

#endif // EIGENKERNEL_H
