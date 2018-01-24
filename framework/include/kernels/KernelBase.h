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

#ifndef KERNELBASE_H
#define KERNELBASE_H

#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "MaterialPropertyInterface.h"
#include "RandomInterface.h"
#include "GeometricSearchInterface.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"

class MooseMesh;
class SubProblem;
class KernelBase;
class Assembly;
class MooseVariable;

template <>
InputParameters validParams<KernelBase>();

/**
 * This is the common base class for the two main
 * kernel types implemented in MOOSE, EigenKernel and Kernel.
 */
class KernelBase : public MooseObject,
                   public BlockRestrictable,
                   public SetupInterface,
                   public CoupleableMooseVariableDependencyIntermediateInterface,
                   public FunctionInterface,
                   public UserObjectInterface,
                   public TransientInterface,
                   public PostprocessorInterface,
                   public VectorPostprocessorInterface,
                   public MaterialPropertyInterface,
                   public RandomInterface,
                   protected GeometricSearchInterface,
                   public Restartable,
                   public ZeroInterface,
                   public MeshChangedInterface
{
public:
  KernelBase(const InputParameters & parameters);

  virtual ~KernelBase();

  /// Compute this Kernel's contribution to the residual
  virtual void computeResidual() = 0;

  /// Compute this Kernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() = 0;

  /// Computes d-residual / d-jvar... storing the result in Ke.
  virtual void computeOffDiagJacobian(unsigned int jvar) = 0;

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) = 0;

  /**
   * Compute this Kernel's contribution to the diagonal Jacobian entries
   * corresponding to nonlocal dofs of the variable
   */
  virtual void computeNonlocalJacobian() {}

  /**
   * Computes d-residual / d-jvar... corresponding to nonlocal dofs of the jvar
   * and stores the result in nonlocal ke
   */
  virtual void computeNonlocalOffDiagJacobian(unsigned int /* jvar */) {}

  /// Returns the variable number that this Kernel operates on.
  MooseVariable & variable();

  /// Returns a reference to the SubProblem for which this Kernel is active
  SubProblem & subProblem();

  virtual bool isEigenKernel() const { return _eigen_kernel; }

protected:
  /// Reference to this kernel's SubProblem
  SubProblem & _subproblem;

  /// Reference to this kernel's FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to the EquationSystem object
  SystemBase & _sys;

  /// The thread ID for this kernel
  THREAD_ID _tid;

  /// Reference to this Kernel's assembly object
  Assembly & _assembly;

  /// Reference to this Kernel's MooseVariable object
  MooseVariable & _var;

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

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariable *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  bool _eigen_kernel;
};

#endif /* KERNELBASE_H */
