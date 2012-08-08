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
#include "Coupleable.h"
#include "MooseVariableInterface.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "DependencyResolverInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseMesh.h"

//forward declarations
class Problem;
class SubProblem;
class AuxKernel;
class AuxiliarySystem;
class SystemBase;

template<>
InputParameters validParams<AuxKernel>();

/**
 * Base class for creating new auxiliary kernels and auxiliary boundary conditions.
 *
 */
class AuxKernel :
  public MooseObject,
  public SetupInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  public DependencyResolverInterface,
  protected GeometricSearchInterface
{
public:
  AuxKernel(const std::string & name, InputParameters parameters);

  virtual ~AuxKernel() {}

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

  virtual
  const std::set<std::string> &
  getRequestedItems() { return _depend_vars; }

  virtual
  const std::set<std::string> &
  getSuppliedItems() { return _supplied_vars; }

  /**
   * Override functions from MaterialPropertyInterface for error checking
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

protected:
  virtual Real computeValue() = 0;

  /// Subproblem this kernel is part of
  SubProblem & _subproblem;
  /// System this kernel is part of
  SystemBase & _sys;
  SystemBase & _nl_sys;
  AuxiliarySystem & _aux_sys;
  /// Thread ID
  THREAD_ID _tid;
  /// Variable this kernel is acting on
  MooseVariable & _var;
  /// true if the kernel is nodal, false if it is elemental
  bool _nodal;
  /// true if the kernel is boundary kernel, false if it is interior kernels
  bool _bnd;
  /// Mesh this kernel is active on
  MooseMesh & _mesh;
  /// Dimension of the problem being solved
  unsigned int _dim;

  /// Active quadrature points
  const MooseArray< Point > & _q_point;
  /// Quadrature rule being used
  QBase * & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
  /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_old;
  /// Holds the t-2 solution at the current quadrature point.
  VariableValue & _u_older;

  /// Current element (valid only for elemental kernels)
  const Elem * & _current_elem;
  /// Volume of the current element
  const Real & _current_elem_volume;
  /// Volume of the current side
  const Real & _current_side_volume;

  /// Current node (valid only for nodal kernels)
  const Node * & _current_node;

  /// reference to the solution vector of auxiliary system
  NumericVector<Number> & _solution;

  /// Quadrature point index
  unsigned int _qp;

  // Single Instance Variables
  /// Scalar zero
  Real & _real_zero;
  /// Scalar zero in quadrature points
  MooseArray<Real> & _zero;
  /// Zero gradient in quadrature points
  MooseArray<RealGradient> & _grad_zero;
  /// Zero second derivative in quadrature points
  MooseArray<RealTensor> & _second_zero;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;
};

template<typename T>
MaterialProperty<T> &
AuxKernel::getMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError(std::string("Nodal AuxKernel '") + _name + "' attempted to reference material property '" + name + "'");
  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

template<typename T>
MaterialProperty<T> &
AuxKernel::getMaterialPropertyOld(const std::string & name)
{
  if (isNodal())
    mooseError(std::string("Nodal AuxKernel '") + _name + "' attempted to reference material property '" + name + "'");
  return MaterialPropertyInterface::getMaterialPropertyOld<T>(name);
}

template<typename T>
MaterialProperty<T> &
AuxKernel::getMaterialPropertyOlder(const std::string & name)
{
  if (isNodal())
    mooseError(std::string("Nodal AuxKernel '") + _name + "' attempted to reference material property '" + name + "'");
  return MaterialPropertyInterface::getMaterialPropertyOlder<T>(name);
}

#endif //AUXKERNEL_H
