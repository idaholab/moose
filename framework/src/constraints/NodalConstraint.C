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

template<>
InputParameters validParams<NodalConstraint>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this constraint is applied to.");
  params.addRequiredParam<unsigned int>("master", "The ID of the master node");
  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_constraint");
  return params;
}

NodalConstraint::NodalConstraint(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  SetupInterface(parameters),
  NeighborCoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
  FunctionInterface(parameters),
  UserObjectInterface(parameters),
  TransientInterface(parameters, name, "nodal_constraints"),
  GeometricSearchInterface(parameters),
  Restartable(name, parameters, "Constraints"),
  _subproblem(*parameters.get<SubProblem *>("_subproblem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _assembly(_subproblem.assembly(_tid)),
  _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
  _mesh(_subproblem.mesh()),
//  _dim(_mesh.dimension()),

  _master_node_id(getParam<unsigned int>("master")),

  _master_node(_assembly.node()),
  _u_master(_var.nodalSln()),
  _slave_node(_assembly.nodeNeighbor()),
  _u_slave(_var.nodalSlnNeighbor())
{
}

NodalConstraint::~NodalConstraint()
{
}

unsigned int
NodalConstraint::getMasterNodeId()
{
  return _master_node_id;
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  Real scaling_factor = _var.scalingFactor();
  _qp = 0;
  // master node
  dof_id_type & dof_idx = _var.nodalDofIndex();
  residual.add(dof_idx, scaling_factor * computeQpResidual(Moose::Master));
  // slave node
  dof_id_type & dof_idx_neighbor = _var.nodalDofIndexNeighbor();
  residual.add(dof_idx_neighbor, scaling_factor * computeQpResidual(Moose::Slave));
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  _qp = 0;
  dof_id_type & dof_idx = _var.nodalDofIndex();
  dof_id_type & dof_idx_neighbor = _var.nodalDofIndexNeighbor();

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
