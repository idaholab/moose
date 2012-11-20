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
#include "NodeFaceConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"
#include "MooseEnum.h"

// libMesh includes
#include "string_to_enum.h"

template<>
InputParameters validParams<NodeFaceConstraint>()
{
  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  MooseEnum orders("FIRST, SECOND, THIRD, FORTH", "FIRST");

  InputParameters params = validParams<MooseObject>();
  params.addParam<MooseEnum>("execute_on", execute_options, "Set to (residual|timestep|timestep_begin|custom) to execute only at that moment");
  params.addRequiredParam<std::string>("variable", "The name of the variable that this constraint is applied to.");
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<BoundaryName>("master", "The boundary ID associated with the master side");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order used for projections");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_constraint");
  return params;
}

NodeFaceConstraint::NodeFaceConstraint(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  SetupInterface(parameters),
  CoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
  FunctionInterface(parameters),
  TransientInterface(parameters, name, "node_face_constraints"),
  GeometricSearchInterface(parameters),

  _subproblem(*parameters.get<SubProblem *>("_subproblem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _assembly(_subproblem.assembly(_tid)),
  _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
  _mesh(_subproblem.mesh()),
  _dim(_mesh.dimension()),

  _slave(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
  _master(_mesh.getBoundaryID(getParam<BoundaryName>("master"))),

  _master_q_point(_subproblem.pointsFace(_tid)),
  _master_qrule(_subproblem.qRuleFace(_tid)),

  _penetration_locator(getPenetrationLocator(getParam<BoundaryName>("master"), getParam<BoundaryName>("slave"),
                                             Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),

  _current_node(_var.node()),
  _current_master(_var.neighbor()),
  _u_slave(_var.nodalSln()),
  _phi_slave(1),  // One entry
  _test_slave(1),  // One entry

  _phi_master(_assembly.phiFaceNeighbor()),
  _grad_phi_master(_assembly.gradPhiFaceNeighbor()),

  _test_master(_var.phiFaceNeighbor()),
  _grad_test_master(_var.gradPhiFaceNeighbor()),

  _u_master(_var.slnNeighbor()),
  _grad_u_master(_var.gradSlnNeighbor()),

  _dof_map(_sys.dofMap()),
  _node_to_elem_map(_mesh.nodeToElemMap()),

  _overwrite_slave_residual(true),

  _real_zero(_subproblem._real_zero[_tid]),
  _zero(_subproblem._zero[_tid]),
  _grad_zero(_subproblem._grad_zero[_tid]),
  _second_zero(_subproblem._second_zero[_tid])
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  // Put a "1" into test_slave
  // will always only have one entry that is 1
  _test_slave[0].push_back(1);
}
void
NodeFaceConstraint::computeSlaveValue(NumericVector<Number> & current_solution)
{
  unsigned int & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSlaveValue());
}

void
NodeFaceConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_var.number());

  _qp=0;

  for (_i=0; _i<_test_master.size(); _i++)
    neighbor_re(_i) += computeQpResidual(Moose::Master);


  _i=0;
  re(0) = computeQpResidual(Moose::Slave);
}

void
NodeFaceConstraint::computeJacobian()
{
  std::vector<unsigned int> & elems = _node_to_elem_map[_current_node->id()];

  _connected_dof_indices.clear();
  std::set<unsigned int> unique_dof_indices;

  // Get the dof indices from each elem connected to the node
  for(unsigned int el=0; el < elems.size(); ++el)
  {
    unsigned int cur_elem = elems[el];

    std::vector<unsigned int> dof_indices;
    _dof_map.dof_indices(_mesh.elem(cur_elem), dof_indices, _var.number());

    for(unsigned int di=0; di < dof_indices.size(); di++)
      unique_dof_indices.insert(dof_indices[di]);
  }

  for(std::set<unsigned int>::iterator sit=unique_dof_indices.begin(); sit != unique_dof_indices.end(); ++sit)
    _connected_dof_indices.push_back(*sit);

  //  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  //  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), _var.number());
  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for(unsigned int j=0; j<_connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if(_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kee(_i,_j) += computeQpJacobian(Moose::SlaveSlave);

  for (_i=0; _i<_test_slave.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Ken(_i,_j) += computeQpJacobian(Moose::SlaveMaster);

  for (_i=0; _i<_test_master.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kne(_i,_j) += computeQpJacobian(Moose::MasterSlave);

  for (_i=0; _i<_test_master.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Knn(_i,_j) += computeQpJacobian(Moose::MasterMaster);
}
