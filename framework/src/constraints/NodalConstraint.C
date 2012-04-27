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
#include "NodalConstraint.h"

#include "SystemBase.h"

// libMesh includes
#include "string_to_enum.h"

template<>
InputParameters validParams<NodalConstraint>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<SetupInterface>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this constraint is applied to.");
  params.addRequiredParam<unsigned int>("master", "The ID of the master node");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_constraint");
  return params;
}

NodalConstraint::NodalConstraint(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  SetupInterface(parameters),
  NeighborCoupleable(parameters, true),
  NeighborMooseVariableInterface(parameters, true),
  FunctionInterface(parameters),
  UserObjectInterface(parameters),
  TransientInterface(parameters),
  GeometricSearchInterface(parameters),

  _problem(*parameters.get<Problem *>("_problem")),
  _subproblem(*parameters.get<SubProblem *>("_subproblem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _assembly(_subproblem.assembly(_tid)),
  _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
  _mesh(_subproblem.mesh()),
  _dim(_mesh.dimension()),

  _master_node_id(getParam<unsigned int>("master")),

  _master_node(_subproblem.node(_tid)),
  _u_master(_var.nodalSln()),
  _slave_node(_subproblem.nodeNeighbor(_tid)),
  _u_slave(_var.nodalSlnNeighbor())
{
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  Real scaling_factor = _var.scalingFactor();
  _qp = 0;
  // master node
  unsigned int & dof_idx = _var.nodalDofIndex();
  residual.add(dof_idx, scaling_factor * computeQpResidual(Moose::Master));
  // slave node
  unsigned int & dof_idx_neighbor = _var.nodalDofIndexNeighbor();
  residual.add(dof_idx_neighbor, scaling_factor * computeQpResidual(Moose::Slave));
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  _qp = 0;
  unsigned int & dof_idx = _var.nodalDofIndex();
  unsigned int & dof_idx_neighbor = _var.nodalDofIndexNeighbor();

  Real scaling_factor = _var.scalingFactor();
  jacobian.add(dof_idx, dof_idx, scaling_factor * computeQpJacobian(Moose::MasterMaster));
  jacobian.add(dof_idx, dof_idx_neighbor, scaling_factor * computeQpJacobian(Moose::MasterSlave));
  jacobian.add(dof_idx_neighbor, dof_idx_neighbor, scaling_factor * computeQpJacobian(Moose::SlaveSlave));
  jacobian.add(dof_idx_neighbor, dof_idx, scaling_factor * computeQpJacobian(Moose::SlaveMaster));
}

void
NodalConstraint::updateConnectivity()
{
}
