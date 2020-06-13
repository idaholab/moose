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
#include "MooseVariableInterface.h"
#include "ElementIDInterface.h"
#include "UserObject.h"

// forward declarations
template <typename ComputeValueType>
class AuxKernelTempl;

typedef AuxKernelTempl<Real> AuxKernel;
typedef AuxKernelTempl<RealVectorValue> VectorAuxKernel;

class SubProblem;
class AuxiliarySystem;
class SystemBase;
class MooseMesh;

template <>
InputParameters validParams<AuxKernel>();

template <>
InputParameters validParams<VectorAuxKernel>();

/**
 * Base class for creating new auxiliary kernels and auxiliary boundary conditions.
 *
 */
template <typename ComputeValueType>
class AuxKernelTempl : public MooseObject,
                       public MooseVariableInterface<ComputeValueType>,
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
                       protected GeometricSearchInterface,
                       public Restartable,
                       public MeshChangedInterface,
                       protected VectorPostprocessorInterface,
                       public ElementIDInterface
{
public:
  static InputParameters validParams();

  AuxKernelTempl(const InputParameters & parameters);

  /**
   * Computes the value and stores it in the solution vector
   */
  virtual void compute();

  /**
   * Nodal or elemental kernel?
   * @return true if this is a nodal kernel, otherwise false
   */
  bool isNodal() const { return _nodal; }

  /**
   * Get a reference to a variable this kernel is action on
   * @return reference to a variable this kernel is action on
   */
  MooseVariableFE<ComputeValueType> & variable() { return _var; }

  const std::set<UserObjectName> & getDependObjects() const { return _depend_uo; }

  void coupledCallback(const std::string & var_name, bool is_old) const override;

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

  /**
   * Override functions from MaterialPropertyInterface for error checking
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

  template <typename T>
  const T & getUserObject(const UserObjectName & name);
  template <typename T>
  const T & getUserObjectByName(const UserObjectName & name);

  const UserObject & getUserObjectBase(const UserObjectName & name);
  const UserObject & getUserObjectBaseByName(const UserObjectName & name);

  virtual const PostprocessorValue & getPostprocessorValue(const std::string & name,
                                                           unsigned int index = 0);
  virtual const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name);

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & name, const std::string & vector_name) override;
  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName &,
                                    const std::string & vector_name) override;

  virtual const VectorPostprocessorValue & getVectorPostprocessorValue(
      const std::string & name, const std::string & vector_name, bool needs_broadcast) override;
  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName &,
                                    const std::string & vector_name,
                                    bool needs_broadcast) override;

  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const std::string & name,
                                     const std::string & vector_name) override;

  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueByName(const std::string & name,
                                           const std::string & vector_name) override;

protected:
  /**
   * Compute and return the value of the aux variable.
   */
  virtual ComputeValueType computeValue() = 0;

  virtual const VariableValue & coupledDot(const std::string & var_name,
                                           unsigned int comp = 0) const override;

  virtual const VariableValue & coupledDotDu(const std::string & var_name,
                                             unsigned int comp = 0) const override;

  /// This callback is used for AuxKernelTempls that need to perform a per-element calculation
  virtual void precalculateValue() {}

  /// Subproblem this kernel is part of
  SubProblem & _subproblem;
  /// System this kernel is part of
  SystemBase & _sys;
  SystemBase & _nl_sys;
  AuxiliarySystem & _aux_sys;

  /// Thread ID
  THREAD_ID _tid;

  /// This is a regular kernel so we cast to a regular MooseVariable
  MooseVariableFE<ComputeValueType> & _var;

  /// Flag indicating if the AuxKernel is nodal
  bool _nodal;

  /// Holds the solution at current quadrature points
  const typename OutputTools<ComputeValueType>::VariableValue & _u;

  /// Holds the previous solution at the current quadrature point.
  const typename OutputTools<ComputeValueType>::VariableValue & _u_old;

  /// Holds the t-2 solution at the current quadrature point.
  const typename OutputTools<ComputeValueType>::VariableValue & _u_older;

  /// Holds the the test functions
  const typename OutputTools<ComputeValueType>::VariableTestValue & _test;

  /// Assembly class
  Assembly & _assembly;

  /// true if the kernel is boundary kernel, false if it is interior kernels
  bool _bnd;
  /// Mesh this kernel is active on
  MooseMesh & _mesh;
  /// Dimension of the problem being solved
  //  unsigned int _dim;

  /// Active quadrature points
  const MooseArray<Point> & _q_point;
  /// Quadrature rule being used
  const QBase * const & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// Current element (valid only for elemental kernels)
  const Elem * const & _current_elem;
  /// current side of the current element
  const unsigned int & _current_side;

  /// Volume of the current element
  const Real & _current_elem_volume;
  /// Volume of the current side
  const Real & _current_side_volume;

  /// Current node (valid only for nodal kernels)
  const Node * const & _current_node;

  /// The current boundary ID
  const BoundaryID & _current_boundary_id;

  /// reference to the solution vector of auxiliary system
  NumericVector<Number> & _solution;

  /// Quadrature point index
  unsigned int _qp;

  /// Depend AuxKernelTempls
  mutable std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects
  std::set<UserObjectName> _depend_uo;

  /// number of local dofs for elemental variables
  unsigned int _n_local_dofs;

  /// for holding local load
  DenseVector<Number> _local_re;
  /// for holding local solution
  DenseVector<Number> _local_sol;
  /// for holding local mass matrix
  DenseMatrix<Number> _local_ke;

  using MooseVariableInterface<ComputeValueType>::mooseVariable;
};

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

template <typename ComputeValueType>
template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
AuxKernelTempl<ComputeValueType>::getGenericMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getGenericMaterialProperty<T, is_ad>(name);
}

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialPropertyOld(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOld<T>(name);
}

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialPropertyOlder(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOlder<T>(name);
}

template <typename ComputeValueType>
template <typename T>
const T &
AuxKernelTempl<ComputeValueType>::getUserObject(const UserObjectName & name)
{
  _depend_uo.insert(_pars.get<UserObjectName>(name));
  auto & uo = UserObjectInterface::getUserObject<T>(name);
  auto indirect_dependents = uo.getDependObjects();
  for (auto & indirect_dependent : indirect_dependents)
    _depend_uo.insert(indirect_dependent);
  return uo;
}

template <typename ComputeValueType>
template <typename T>
const T &
AuxKernelTempl<ComputeValueType>::getUserObjectByName(const UserObjectName & name)
{
  _depend_uo.insert(name);
  auto & uo = UserObjectInterface::getUserObjectByName<T>(name);
  auto indirect_dependents = uo.getDependObjects();
  for (auto & indirect_dependent : indirect_dependents)
    _depend_uo.insert(indirect_dependent);
  return uo;
}
