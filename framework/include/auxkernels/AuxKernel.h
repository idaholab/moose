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

#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "MooseObject.h"
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
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"
#include "VectorPostprocessorInterface.h"

// forward declarations
class SubProblem;
class AuxKernel;
class AuxiliarySystem;
class SystemBase;
class MooseMesh;

template <>
InputParameters validParams<AuxKernel>();

/**
 * Base class for creating new auxiliary kernels and auxiliary boundary conditions.
 *
 */
class AuxKernel : public MooseObject,
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
                  public ZeroInterface,
                  public MeshChangedInterface,
                  protected VectorPostprocessorInterface
{
public:
  AuxKernel(const InputParameters & parameters);

  virtual ~AuxKernel();

  /**
   * Computes the value and stores it in the solution vector
   */
  virtual void compute();

  /**
   * Get a reference to a variable this kernel is action on
   * @return reference to a variable this kernel is action on
   */
  MooseVariable & variable() { return _var; }

  /**
   * Nodal or elemental kernel?
   * @return true if this is a nodal kernel, otherwise false
   */
  bool isNodal();

  const std::set<std::string> & getDependObjects() const { return _depend_uo; }

  void coupledCallback(const std::string & var_name, bool is_old) override;

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

  /**
   * Override functions from MaterialPropertyInterface for error checking
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

  template <typename T>
  const T & getUserObject(const std::string & name);
  template <typename T>
  const T & getUserObjectByName(const UserObjectName & name);

  const UserObject & getUserObjectBase(const std::string & name);

  virtual const PostprocessorValue & getPostprocessorValue(const std::string & name);
  virtual const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name);

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & name, const std::string & vector_name) override;
  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName &,
                                    const std::string & vector_name) override;

protected:
  virtual const VariableValue & coupledDot(const std::string & var_name,
                                           unsigned int comp = 0) override;

  virtual const VariableValue & coupledDotDu(const std::string & var_name,
                                             unsigned int comp = 0) override;

  virtual Real computeValue() = 0;

  /// This callback is used for AuxKernels that need to perform a per-element calculation
  virtual void precalculateValue() {}

  /// Subproblem this kernel is part of
  SubProblem & _subproblem;
  /// System this kernel is part of
  SystemBase & _sys;
  SystemBase & _nl_sys;
  AuxiliarySystem & _aux_sys;
  /// Thread ID
  THREAD_ID _tid;
  /// Assembly class
  Assembly & _assembly;
  /// Variable this kernel is acting on
  MooseVariable & _var;
  /// true if the kernel is nodal, false if it is elemental
  bool _nodal;
  /// true if the kernel is boundary kernel, false if it is interior kernels
  bool _bnd;
  /// Mesh this kernel is active on
  MooseMesh & _mesh;
  /// Dimension of the problem being solved
  //  unsigned int _dim;

  /// Active quadrature points
  const MooseArray<Point> & _q_point;
  /// Quadrature rule being used
  QBase *& _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the previous solution at the current quadrature point.
  const VariableValue & _u_old;
  /// Holds the t-2 solution at the current quadrature point.
  const VariableValue & _u_older;
  /// holds the the test functions
  const VariableTestValue & _test;

  /// Current element (valid only for elemental kernels)
  const Elem *& _current_elem;
  /// current side of the current element
  unsigned int & _current_side;

  /// Volume of the current element
  const Real & _current_elem_volume;
  /// Volume of the current side
  const Real & _current_side_volume;

  /// Current node (valid only for nodal kernels)
  const Node *& _current_node;

  /// reference to the solution vector of auxiliary system
  NumericVector<Number> & _solution;

  /// Quadrature point index
  unsigned int _qp;

  /// Depend AuxKernels
  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects
  std::set<std::string> _depend_uo;

  /// number of local dofs for elemental variables
  unsigned int _n_local_dofs;

  /// for holding local load
  DenseVector<Number> _local_re;
  /// for holding local solution
  DenseVector<Number> _local_sol;
  /// for holding local mass matrix
  DenseMatrix<Number> _local_ke;
};

template <typename T>
const MaterialProperty<T> &
AuxKernel::getMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernel::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

template <typename T>
const MaterialProperty<T> &
AuxKernel::getMaterialPropertyOld(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernel::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOld<T>(name);
}

template <typename T>
const MaterialProperty<T> &
AuxKernel::getMaterialPropertyOlder(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernel::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOlder<T>(name);
}

template <typename T>
const T &
AuxKernel::getUserObject(const std::string & name)
{
  _depend_uo.insert(_pars.get<UserObjectName>(name));
  return UserObjectInterface::getUserObject<T>(name);
}

template <typename T>
const T &
AuxKernel::getUserObjectByName(const UserObjectName & name)
{
  _depend_uo.insert(name);
  return UserObjectInterface::getUserObjectByName<T>(name);
}

#endif // AUXKERNEL_H
