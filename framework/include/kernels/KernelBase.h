//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"

class MooseMesh;
class SubProblem;
class KernelBase;
class Assembly;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;

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
                   public MeshChangedInterface,
                   public TaggingInterface
{
public:
  KernelBase(const InputParameters & parameters);

  virtual ~KernelBase();

  /// Compute this Kernel's contribution to the residual
  virtual void computeResidual() = 0;

  /// Compute this Kernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() = 0;

  /// Computes d-residual / d-jvar... storing the result in Ke.
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) = 0;

  virtual void computeADOffDiagJacobian()
  {
    mooseError("The computeADOffDiagJacobian method should only be called on ADKernel objects");
  }

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

  /**
   * Returns the variable number that this Kernel operates on.
   */
  virtual MooseVariableFEBase & variable() = 0;

  /**
   * Returns a reference to the SubProblem for which this Kernel is active
   */
  SubProblem & subProblem() { return _subproblem; }

protected:
  /**
   * Compute this Kernel's contribution to the Jacobian at the current quadrature point
   */
  virtual Real computeQpJacobian() { return 0; }
  /**
   * This is the virtual that derived classes should override for computing an off-diagonal Jacobian
   * component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }

  /**
   * Following methods are used for Kernels that need to perform a per-element calculation
   */
  virtual void precalculateResidual() {}
  virtual void precalculateJacobian() {}
  virtual void precalculateOffDiagJacobian(unsigned int /* jvar */) {}

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

  /// Reference to this Kernel's mesh object
  MooseMesh & _mesh;

  const Elem * const & _current_elem;

  /// Volume of the current element
  const Real & _current_elem_volume;

  /// The current quadrature point index
  unsigned int _qp;

  /// The physical location of the element's quadrature Points, indexed by _qp
  const MooseArray<Point> & _q_point;

  /// active quadrature rule
  const QBase * const & _qrule;

  /// The current quadrature point weight value
  const MooseArray<Real> & _JxW;

  /// The scaling factor to convert from cartesian to another coordinate system (e.g rz, spherical, etc.)
  const MooseArray<Real> & _coord;

  /// current index for the test function
  unsigned int _i;

  /// current index for the shape function
  unsigned int _j;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  std::vector<unsigned int> _displacements;
};

