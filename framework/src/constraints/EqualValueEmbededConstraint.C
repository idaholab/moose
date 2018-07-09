//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EqualValueEmbededConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "AuxiliarySystem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("MooseApp", EqualValueEmbededConstraint);

template <>
InputParameters
validParams<EqualValueEmbededConstraint>()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  InputParameters params = validParams<NodeElemConstraint>();
  params.addClassDescription("This is a constraint enforcing overlapping portions of two blocks to "
                             "have the same variable value");
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam<std::string>("model", "glued", "The contact model to use");
  params.addParam<std::string>("formulation", "default", "The contact formulation");
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  return params;
}

EqualValueEmbededConstraint::EqualValueEmbededConstraint(const InputParameters & parameters)
  : NodeElemConstraint(parameters),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _model(contactModel(getParam<std::string>("model"))),
    _formulation(contactFormulation(getParam<std::string>("formulation"))),
    _penalty(getParam<Real>("penalty")),
    _residual_copy(_sys.residualGhosted()),
    _mesh_dimension(_mesh.dimension())
{
  _overwrite_slave_residual = false;
}

void
EqualValueEmbededConstraint::timestepSetup()
{
}

void
EqualValueEmbededConstraint::jacobianSetup()
{
}

bool
EqualValueEmbededConstraint::shouldApply()
{
  if (_current_elem != NULL)
  {
    // This computes the contact force once per constraint per variable
    computeContactForce();
    return true;
  }
  return false;
}

void
EqualValueEmbededConstraint::computeContactForce()
{
  const Node * node = _current_node;
  unsigned int sys_num = _sys.number();
  dof_id_type dof_number = node->dof_number(sys_num, _var.number(), 0);

  switch (_model)
  {
    case CM_GLUED:
      switch (_formulation)
      {
        case CF_KINEMATIC:
          _contact_force = -_residual_copy(dof_number);
          break;

        case CF_PENALTY:
          _contact_force = _penalty * (_u_slave[0] - _u_master[0]);
          break;

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;

    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }
}

Real
EqualValueEmbededConstraint::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

Real
EqualValueEmbededConstraint::computeQpResidual(Moose::ConstraintType type)
{
  Real resid = _contact_force;

  switch (type)
  {
    case Moose::Slave:
    {
      if (_formulation == CF_KINEMATIC)
      {
        Real pen_force = _penalty * (_u_slave[_qp] - _u_master[_qp]);
        if (_model == CM_GLUED)
          resid += pen_force;
      }
      return _test_slave[_i][_qp] * resid;
    }

    case Moose::Master:
      return _test_master[_i][_qp] * -resid;
  }

  return 0.0;
}

Real
EqualValueEmbededConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  unsigned int sys_num = _sys.number();
  const Real penalty = _penalty;

  switch (type)
  {
    case Moose::SlaveSlave:
      switch (_model)
      {
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real curr_jac = (*_jacobian)(
                  _current_node->dof_number(sys_num, _var.number(), 0), _connected_dof_indices[_j]);
              return -curr_jac + _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];
              // return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];
            }

            case CF_PENALTY:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];
            default:
              mooseError("Invalid contact formulation");
          }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SlaveMaster:
      switch (_model)
      {
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];
            }

            case CF_PENALTY:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];
            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterSlave:
      switch (_model)
      {
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real slave_jac = (*_jacobian)(
                  _current_node->dof_number(sys_num, _var.number(), 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }

            case CF_PENALTY:
              return -_phi_slave[_j][_qp] * penalty * _test_master[_i][_qp];
            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterMaster:
      switch (_model)
      {
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
              return 0.0;
            case CF_PENALTY:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp];
            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

Real
EqualValueEmbededConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                      unsigned int /*jvar*/)
{
  unsigned int sys_num = _sys.number();
  switch (type)
  {
    case Moose::SlaveSlave:
      switch (_model)
      {
        case CM_GLUED:
        {
          const Real curr_jac = (*_jacobian)(_current_node->dof_number(sys_num, _var.number(), 0),
                                             _connected_dof_indices[_j]);
          return -curr_jac;
        }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SlaveMaster:
      switch (_model)
      {
        case CM_GLUED:
          return 0;

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterSlave:
      switch (_model)
      {
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real slave_jac = (*_jacobian)(
                  _current_node->dof_number(sys_num, _var.number(), 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }
            case CF_PENALTY:
              return 0.0;
            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterMaster:
      switch (_model)
      {
        case CM_GLUED:
          return 0.0;

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

void
EqualValueEmbededConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SlaveSlave);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());
  if (Ken.m() && Ken.n())
    for (_i = 0; _i < _test_slave.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Ken(_i, _j) += computeQpJacobian(Moose::SlaveMaster);

  _Kne.resize(_test_master.size(), _connected_dof_indices.size());
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
EqualValueEmbededConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);

  _Kne.resize(_test_master.size(), _connected_dof_indices.size());
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
EqualValueEmbededConstraint::getConnectedDofIndices(unsigned int var_num)
{
  NodeElemConstraint::getConnectedDofIndices(var_num);

  _phi_slave.resize(_connected_dof_indices.size());

  dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();

  // Fill up _phi_slave so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  _qp = 0;
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }
}

void
EqualValueEmbededConstraint::residualEnd()
{
}

ContactModel
EqualValueEmbededConstraint::contactModel(std::string name)
{
  ContactModel model(CM_INVALID);
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if ("frictionless" == name)
    model = CM_FRICTIONLESS;
  else if ("glued" == name)
    model = CM_GLUED;
  else if ("coulomb" == name)
    model = CM_COULOMB;
  else
    ::mooseError("Invalid contact model found: ", name);

  return model;
}

ContactFormulation
EqualValueEmbededConstraint::contactFormulation(std::string name)
{
  ContactFormulation formulation(CF_INVALID);
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if ("default" == name || "kinematic" == name)
    formulation = CF_DEFAULT;

  else if ("penalty" == name)
    formulation = CF_PENALTY;

  else if ("augmented_lagrange" == name)
    formulation = CF_AUGMENTED_LAGRANGE;

  else if ("tangential_penalty" == name)
    formulation = CF_TANGENTIAL_PENALTY;

  if (formulation == CF_INVALID)
    ::mooseError("Invalid formulation found: ", name);

  return formulation;
}
