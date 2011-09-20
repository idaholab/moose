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

// libMesh includes
#include "string_to_enum.h"

template<>
InputParameters validParams<NodeFaceConstraint>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this constraint is applied to.");
  params.addRequiredParam<unsigned int>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<unsigned int>("master", "The boundary ID associated with the master side");
  params.addParam<std::string>("order", "FIRST", "The finite element order used for projections");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_constraint");
  return params;
}

NodeFaceConstraint::NodeFaceConstraint(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  Coupleable(parameters, true),
  FunctionInterface(parameters),
  TransientInterface(parameters),
  GeometricSearchInterface(parameters),

  _slave(getParam<unsigned int>("slave")),
  _master(getParam<unsigned int>("master")),

  _problem(*parameters.get<Problem *>("_problem")),
  _subproblem(*parameters.get<SubProblemInterface *>("_subproblem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _asmb(_subproblem.asmBlock(_tid)),
  _asm_data(_subproblem.assembly(_tid)),
  _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
  _mesh(_subproblem.mesh()),
  _dim(_mesh.dimension()),

  _master_q_point(_subproblem.pointsFace(_tid)),
  _master_qrule(_subproblem.qRuleFace(_tid)),

  _penetration_locator(getPenetrationLocator(_master, _slave, Utility::string_to_enum<Order>(getParam<std::string>("order")))),

  _current_node(_var.node()),
  _current_master(_var.neighbor()),
  _u_slave(_var.nodalSln()),
  _phi_slave(1),  // One entry
  _test_slave(1),  // One entry

  _phi_master(_asmb.phiFaceNeighbor()),
  _grad_phi_master(_asmb.gradPhiFaceNeighbor()),
  _second_phi_master(_asmb.secondPhiFaceNeighbor()),

  _test_master(_var.phiFaceNeighbor()),
  _grad_test_master(_var.gradPhiFaceNeighbor()),
  _second_test_master(_var.secondPhiFaceNeighbor()),

  _u_master(_var.slnNeighbor()),
  _u_old_master(_var.slnOldNeighbor()),
  _u_older_master(_var.slnOlderNeighbor()),

  _grad_u_master(_var.gradSlnNeighbor()),
  _grad_u_old_master(_var.gradSlnOldNeighbor()),
  _grad_u_older_master(_var.gradSlnOlderNeighbor()),

  _second_u_master(_var.secondSlnNeighbor()),
  _dof_map(_sys.dofMap()),
  _node_to_elem_map(_mesh.nodeToElemMap())
{
  // Put a "1" into phi_slave and test_slave
  // These will always only have one entry that is 1
  _phi_slave[0].push_back(1);
  _test_slave[0].push_back(1);
}

void
NodeFaceConstraint::computeResidual()
{
//  std::cerr<<"Here!"<<std::endl;
  DenseVector<Number> & re = _asmb.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _asmb.residualBlockNeighbor(_var.number());
//  neighbor_re.zero();
//  re.zero();

//  for (_qp = 0; _qp < _master_qrule->n_points(); _qp++)
//  {
//    _current_point=_physical_point[_qp];
//    if(isActiveAtPoint(_current_elem, _current_point))
//    {
    // Only one of these because we are computing at _one_ node.
//  std::cerr<<neighbor_re.size()<<std::endl;
  _qp=0;

  for (_i=0; _i<_test_master.size(); _i++)
    neighbor_re(_i) += computeQpResidual(Moose::Master);

//  std::cerr<<"Not Here!"<<std::endl;
//  std::cout<<neighbor_re<<std::endl;

  _i=0;


//  std::cout<<re.size()<<std::endl;

  re(0) = computeQpResidual(Moose::Slave);

//  }
}

void
NodeFaceConstraint::computeJacobian()
{
  std::vector<unsigned int> & elems = _node_to_elem_map[_current_node->id()];

  _connected_dof_indices.clear();
  std::set<unsigned int> unique_dof_indices;

  // Get the dof indices from each elem connected to the node
  //
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

  DenseMatrix<Number> & Kee = _asmb.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & Ken = _asmb.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  DenseMatrix<Number> & Kne = _asmb.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), _var.number());
  DenseMatrix<Number> & Knn = _asmb.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.number(), _var.number());

  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _qp = 0;

  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j=0; _j<_phi_slave.size(); _j++)
      Kee(_i,_j) += computeQpJacobian(Moose::SlaveSlave);

  for (_i=0; _i<_test_slave.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Ken(_i,_j) += computeQpJacobian(Moose::SlaveMaster);

  for (_i=0; _i<_test_master.size(); _i++)
//    for (_j=0; _j<_phi_slave.size(); _j++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kne(_i,_j) += computeQpJacobian(Moose::MasterSlave);

  for (_i=0; _i<_test_master.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Knn(_i,_j) += computeQpJacobian(Moose::MasterMaster);
}
