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
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<NodeFaceConstraint>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<BoundaryName>("master",
                                        "The boundary ID associated with the master side");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order used for projections");

  params.addRequiredCoupledVar("master_variable", "The variable on the master side of the domain");

  return params;
}

NodeFaceConstraint::NodeFaceConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    // The slave side is at nodes (hence passing 'true').  The neighbor side is the master side and
    // it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    _slave(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
    _master(_mesh.getBoundaryID(getParam<BoundaryName>("master"))),

    _master_q_point(_assembly.qPointsFace()),
    _master_qrule(_assembly.qRuleFace()),

    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("master"),
                              getParam<BoundaryName>("slave"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),

    _current_node(_var.node()),
    _current_master(_var.neighbor()),
    _u_slave(_var.nodalSln()),
    _phi_slave(1),  // One entry
    _test_slave(1), // One entry

    _master_var(*getVar("master_variable", 0)),
    _master_var_num(_master_var.number()),

    _phi_master(_assembly.phiFaceNeighbor()),
    _grad_phi_master(_assembly.gradPhiFaceNeighbor()),

    _test_master(_var.phiFaceNeighbor()),
    _grad_test_master(_var.gradPhiFaceNeighbor()),

    _u_master(_master_var.slnNeighbor()),
    _grad_u_master(_master_var.gradSlnNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_slave_residual(true)
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }
  if (parameters.isParamValid("normal_smoothing_method"))
  {
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
  }
  // Put a "1" into test_slave
  // will always only have one entry that is 1
  _test_slave[0].push_back(1);
}

NodeFaceConstraint::~NodeFaceConstraint()
{
  _phi_slave.release();
  _test_slave.release();
}

void
NodeFaceConstraint::computeSlaveValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSlaveValue());
}

void
NodeFaceConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_master_var.number());

  _qp = 0;

  for (_i = 0; _i < _test_master.size(); _i++)
    neighbor_re(_i) += computeQpResidual(Moose::Master);

  _i = 0;
  re(0) = computeQpResidual(Moose::Slave);
}

void
NodeFaceConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  //  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  //  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement,
  //  _var.number(), _var.number());
  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SlaveSlave);

  if (Ken.m() && Ken.n())
    for (_i = 0; _i < _test_slave.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Ken(_i, _j) += computeQpJacobian(Moose::SlaveMaster);

  for (_i = 0; _i < _test_master.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += computeQpJacobian(Moose::MasterSlave);

  if (Knn.m() && Knn.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Knn(_i, _j) += computeQpJacobian(Moose::MasterMaster);
}

void
NodeFaceConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);

  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);

  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::MasterMaster, jvar);
}

void
NodeFaceConstraint::getConnectedDofIndices(unsigned int var_num)
{
  MooseVariable & var = _sys.getVariable(0, var_num);

  _connected_dof_indices.clear();
  std::set<dof_id_type> unique_dof_indices;

  auto node_to_elem_pair = _node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != _node_to_elem_map.end(), "Missing entry in node to elem map");
  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  // Get the dof indices from each elem connected to the node
  for (const auto & cur_elem : elems)
  {
    std::vector<dof_id_type> dof_indices;

    var.getDofIndices(_mesh.elemPtr(cur_elem), dof_indices);

    for (const auto & dof : dof_indices)
      unique_dof_indices.insert(dof);
  }

  for (const auto & dof : unique_dof_indices)
    _connected_dof_indices.push_back(dof);
}

bool
NodeFaceConstraint::overwriteSlaveResidual()
{
  return _overwrite_slave_residual;
}
