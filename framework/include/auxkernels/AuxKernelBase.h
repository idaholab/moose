//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseVariableFE.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "DependencyResolverInterface.h"
#include "RandomInterface.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "VectorPostprocessorInterface.h"
#include "ElementIDInterface.h"
#include "UserObject.h"
#include "NonADFunctorInterface.h"

// forward declarations
class SubProblem;
class AuxiliarySystem;
class SystemBase;
class MooseMesh;

/**
 * Base class for auxiliary kernels
 */
class AuxKernelBase : public MooseObject,
                      public BlockRestrictable,
                      public BoundaryRestrictable,
                      public SetupInterface,
                      public CoupleableMooseVariableDependencyIntermediateInterface,
                      public FunctionInterface,
                      public UserObjectInterface,
                      public TransientInterface,
                      public MaterialPropertyInterface,
                      public PostprocessorInterface,
                      public DependencyResolverInterface,
                      public RandomInterface,
                      public GeometricSearchInterface,
                      public Restartable,
                      public MeshChangedInterface,
                      protected VectorPostprocessorInterface,
                      public ElementIDInterface,
                      protected NonADFunctorInterface
{
public:
  static InputParameters validParams();

  AuxKernelBase(const InputParameters & parameters);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  AuxKernelBase(const AuxKernelBase & object, const Moose::Kokkos::FunctorCopy & key);
#endif

  virtual void compute() = 0;

  const std::set<UserObjectName> & getDependObjects() const { return _depend_uo; }

  void coupledCallback(const std::string & var_name, bool is_old) const override;

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

protected:
  /// Base MooseVariable
  MooseVariableFieldBase & _var;

  /// true if the kernel is boundary kernel, false if it is interior kernels
  const bool _bnd;

  /**
   * Whether or not to check for repeated element sides on the sideset to which
   * the auxkernel is restricted (if boundary restricted _and_ elemental). Setting
   * this to false will allow an element with more than one face on the boundary
   * to which it is restricted allow contribution to the element's value(s). This
   * flag allows auxkernels that evaluate boundary-restricted elemental auxvariables
   * to have more than one element face on the boundary of interest.
   */
  const bool _check_boundary_restricted;

  /// Whether we are computing for a lower dimensional variable using boundary restriction, e.g. a
  /// variable whose block restriction is coincident with a higher-dimensional boundary face
  const bool _coincident_lower_d_calc;

  /// Subproblem this kernel is part of
  SubProblem & _subproblem;
  /// System this kernel is part of
  SystemBase & _sys;
  SystemBase & _nl_sys;
  AuxiliarySystem & _aux_sys;

  /// Thread ID
  const THREAD_ID _tid;

  /// Assembly class
  Assembly & _assembly;

  /// Mesh this kernel is active on
  MooseMesh & _mesh;

private:
  void addPostprocessorDependencyHelper(const PostprocessorName & name) const override final;
  void addUserObjectDependencyHelper(const UserObject & uo) const override final;
  void
  addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const override final;

  /// Get MooseVariable of base type from parameter
  MooseVariableFieldBase & getVariableHelper(const InputParameters & parameters);

  /// Depend AuxKernelTempls
  mutable std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects
  mutable std::set<UserObjectName> _depend_uo;
};
