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
#include "OutputInterface.h"
#include "SetupInterface.h"
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseError.h"

#include "libmesh/fe_type.h"
#include "libmesh/enum_fe_family.h"

// libMesh forward declarations
namespace libMesh
{
class DofMap;
class Variable;
}

class Assembly;
class SubProblem;
class SystemBase;
class MooseMesh;

class MooseVariableBase : public MooseObject,
                          public BlockRestrictable,
                          public OutputInterface,
                          public SetupInterface
{
public:
  static InputParameters validParams();

  MooseVariableBase(const InputParameters & parameters);

  /**
   * Get variable number coming from libMesh
   * @return the libmesh variable number
   */
  unsigned int number() const { return _var_num; }

  /**
   * Get the type of finite element object
   */
  const FEType & feType() const { return _fe_type; }

  /**
   * Get the system this variable is part of.
   */
  SystemBase & sys() { return _sys; }

  /**
   * Get the system this variable is part of.
   */
  const SystemBase & sys() const { return _sys; }

  /**
   * Get the variable name
   */
  const std::string & name() const override { return _var_name; }

  /**
   * Get dual mortar option
   */
  bool useDual() const { return _use_dual; }

  /**
   * Get all global dofindices for the variable
   */
  const std::vector<dof_id_type> & allDofIndices() const;
  unsigned int totalVarDofs() { return allDofIndices().size(); }

  /**
   * Kind of the variable (Nonlinear, Auxiliary, ...)
   */
  Moose::VarKindType kind() const { return _var_kind; }

  /**
   * Set the scaling factor for this variable
   */
  void scalingFactor(Real factor);
  void scalingFactor(const std::vector<Real> & factor);

  /**
   * Get the scaling factor for this variable
   */
  Real scalingFactor() const { return _scaling_factor[0]; }
  const std::vector<Real> & arrayScalingFactor() const { return _scaling_factor; }

  /**
   * Get the order of this variable
   * Note: Order enum can be implicitly converted to unsigned int.
   */
  Order order() const;

  /**
   * Get the number of components
   * Note: For standard and vector variables, the number is one.
   */
  unsigned int count() const { return _count; }

  /**
   * Is this variable nodal
   * @return true if it nodal, otherwise false
   */
  virtual bool isNodal() const { mooseError("Base class cannot determine this"); }

  /**
   * Does this variable have DoFs on nodes
   * @return true if it does, false if not.
   */
  virtual bool hasDoFsOnNodes() const { mooseError("Base class cannot determine this"); };

  /**
   * Return the continuity of this variable
   */
  virtual FEContinuity getContinuity() const { mooseError("Base class cannot determine this"); };

  /**
   * The DofMap associated with the system this variable is in.
   */
  const DofMap & dofMap() const { return _dof_map; }

  virtual void getDofIndices(const Elem * /*elem*/,
                             std::vector<dof_id_type> & /*dof_indices*/) const
  {
    mooseError("not implemented");
  };

  /**
   * Get local DoF indices
   */
  virtual const std::vector<dof_id_type> & dofIndices() const { return _dof_indices; }

  /**
   * Obtain DoF indices of a component with the indices of the 0th component
   */
  std::vector<dof_id_type> componentDofIndices(const std::vector<dof_id_type> & dof_indices,
                                               unsigned int component) const;

  /**
   * Get the number of local DoFs
   */
  virtual unsigned int numberOfDofs() const { return _dof_indices.size(); }

  /**
   * Whether or not this variable operates on an eigen kernel
   */
  bool eigen() const { return _is_eigen; }

  /**
   * Mark this variable as an eigen var or non-eigen var
   */
  void eigen(bool eigen) { _is_eigen = eigen; }

  void initialSetup() override;

  virtual void clearAllDofIndices() { _dof_indices.clear(); }

  /**
   * Set the active vector tags
   * @param vtags Additional vector tags that this variable will need to query at dof indices for,
   * in addition to our own required solution tags
   */
  virtual void setActiveTags(const std::set<TagID> & vtags);

  /**
   * @return whether this is an array variable
   */
  bool isArray() const { return _is_array; }

protected:
  /// System this variable is part of
  SystemBase & _sys;

  /// The FEType associated with this variable
  FEType _fe_type;

  /// variable number (from libMesh)
  unsigned int _var_num;

  /// variable number within MOOSE
  unsigned int _index;

  /// Whether or not this variable operates on eigen kernels
  bool _is_eigen;

  /// Variable type (see MooseTypes.h)
  Moose::VarKindType _var_kind;

  /// Problem this variable is part of
  SubProblem & _subproblem;

  /// libMesh variable object for this variable
  const Variable & _variable;

  /// Assembly data
  Assembly & _assembly;

  /// DOF map
  const DofMap & _dof_map;

  /// DOF indices
  std::vector<dof_id_type> _dof_indices;

  /// mesh the variable is active in
  MooseMesh & _mesh;

  /// Thread ID
  THREAD_ID _tid;

  /// Number of variables in the array
  const unsigned int _count;

  /// scaling factor for this variable
  std::vector<Real> _scaling_factor;

  /// Variable name
  std::string _var_name;

  /// If dual mortar approach is used
  bool _use_dual;

  /// Whether this is an array variable
  const bool _is_array;
};

inline void
MooseVariableBase::setActiveTags(const std::set<TagID> &)
{
  mooseError("setActiveTags must be overridden in derived classes.");
}

#define usingMooseVariableBaseMembers                                                              \
  using MooseVariableBase::_sys;                                                                   \
  using MooseVariableBase::_fe_type;                                                               \
  using MooseVariableBase::_var_num;                                                               \
  using MooseVariableBase::_index;                                                                 \
  using MooseVariableBase::_var_kind;                                                              \
  using MooseVariableBase::_subproblem;                                                            \
  using MooseVariableBase::_variable;                                                              \
  using MooseVariableBase::_assembly;                                                              \
  using MooseVariableBase::_dof_map;                                                               \
  using MooseVariableBase::_dof_indices;                                                           \
  using MooseVariableBase::_mesh;                                                                  \
  using MooseVariableBase::_tid;                                                                   \
  using MooseVariableBase::_count;                                                                 \
  using MooseVariableBase::_scaling_factor;                                                        \
  using MooseVariableBase::_var_name
