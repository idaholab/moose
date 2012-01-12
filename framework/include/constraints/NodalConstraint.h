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

#ifndef NODALCONSTRAINT_H
#define NODALCONSTRAINT_H

//MOOSE includes
#include "Moose.h"
#include "MooseObject.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"

//libMesh includes
#include "libmesh_common.h"
#include "elem.h"
#include "point.h"

//Forward Declarations
class Assembly;

class NodalConstraint;

template<>
InputParameters validParams<NodalConstraint>();

/**
 */
class NodalConstraint :
  public MooseObject,
  public SetupInterface,
  public Coupleable,
  public NeighborCoupleable,
  public FunctionInterface,
  public TransientInterface,
  protected GeometricSearchInterface
{
public:
  NodalConstraint(const std::string & name, InputParameters parameters);
  virtual ~NodalConstraint() {}

  /**
   * Get the node ID of the master
   * @return node ID
   */
  unsigned int getMasterNodeId() { return _master_node_id; }

  /**
   * Get the list of connected slave nodes
   * @return list of slave node IDs
   */
  std::vector<unsigned int> & getSlaveNodeId() { return _connected_nodes; }

  /**
   * The variable number that this object operates on.
   */
  MooseVariable & variable() { return _var; }

  /**
   * Subproblem this constriant is part of
   * @return The reference to the subproblem
   */
  SubProblem & subProblem() { return _subproblem; }

  /**
   * Built the connectivity for this constraint
   */
  virtual void updateConnectivity();

  /**
   * Computes the nodal residual.
   */
  virtual void computeResidual(NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;


//  unsigned int _boundary_id;                                            ///< Boundary ID of the master node

  Problem & _problem;
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  unsigned int _i, _j;
  unsigned int _qp;

  unsigned int _master_node_id;                                         ///< master node id

  const Node * & _master_node;                                          ///< Holds the reference to the master node
  VariableValue & _u_master;                                            ///< Holds the current solution at the current quadrature point
  const Node * & _slave_node;                                           ///< Holds the reference to the current slave node
  VariableValue & _u_slave;                                             ///< Value of the unknown variable this BC is action on

  std::vector<unsigned int> _connected_nodes;                           ///< node IDs connected to the master node (slave nodes)
};

#endif
