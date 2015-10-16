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

#ifndef NODALKERNEL_H
#define NODALKERNEL_H

// MOOSE
#include "MooseObject.h"
#include "SetupInterface.h"
#include "MooseVariable.h"
#include "ParallelUniqueId.h"
#include "MooseArray.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

// libMesh
#include "libmesh/elem.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/numeric_vector.h"

// Forward declerations
class MooseVariable;
class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;
class NodalKernel;

template<>
InputParameters validParams<NodalKernel>();

/**
 * Base class for creating new types of boundary conditions
 *
 */
class NodalKernel :
  public MooseObject,
  public BlockRestrictable,
  public SetupInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public GeometricSearchInterface,
  public Restartable,
  public ZeroInterface,
  public MeshChangedInterface,
  public RandomInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface
{
public:

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  NodalKernel(const InputParameters & parameters);

  /**
   * Gets the variable this is active on
   * @return the variable
   */
  MooseVariable & variable();

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  SubProblem & subProblem();

  /**
   * Compute the residual at the current node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpResidual()
   */
  virtual void computeResidual();

  /**
   * Compute the Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpJacobian()
   */
  virtual void computeJacobian();

  /**
   * Compute the off-diagonal Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpOffDiagJacobian()
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /**
   * The user can override this function to compute the residual at a node.
   */
  virtual Real computeQpResidual() = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution.  If not overriden,
   * returns 1.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Reference to SubProblem
  SubProblem & _subproblem;

  /// Reference to FEProblem
  FEProblem & _fe_problem;

  /// Reference to SystemBase
  SystemBase & _sys;

  /// Thread id
  THREAD_ID _tid;

  /// Reference to assembly
  Assembly & _assembly;

  /// variable this works on
  MooseVariable & _var;

  /// Mesh this is defined on
  MooseMesh & _mesh;

  /// current node being processed
  const Node * & _current_node;

  /// Quadrature point index
  unsigned int _qp;

  /// Value of the unknown variable this is acting on
  VariableValue & _u;

  /// Time derivative of the variable this is acting on
  VariableValue & _u_dot;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable*> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariable*> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;
};

#endif /* NODALKERNEL_H */
