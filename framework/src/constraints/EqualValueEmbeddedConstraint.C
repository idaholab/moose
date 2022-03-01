//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EqualValueEmbeddedConstraint.h"
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

registerMooseObject("MooseApp", EqualValueEmbeddedConstraint);

InputParameters
EqualValueEmbeddedConstraint::validParams()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  InputParameters params = NodeElemConstraint::validParams();
  params.addClassDescription("This is a constraint enforcing overlapping portions of two blocks to "
                             "have the same variable value");
  params.set<bool>("use_displaced_mesh") = false;
  MooseEnum formulation("kinematic penalty", "kinematic");
  params.addParam<MooseEnum>(
      "formulation", formulation, "Formulation used to enforce the constraint");
  params.addRequiredParam<Real>(
      "penalty",
      "Penalty parameter used in constraint enforcement for kinematic and penalty formulations.");

  return params;
}

EqualValueEmbeddedConstraint::EqualValueEmbeddedConstraint(const InputParameters & parameters)
  : NodeElemConstraint(parameters),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _formulation(getParam<MooseEnum>("formulation").getEnum<Formulation>()),
    _penalty(getParam<Real>("penalty")),
    _residual_copy(_sys.residualGhosted())
{
  _overwrite_secondary_residual = false;
  prepareSecondaryToPrimaryMap();
}

void
EqualValueEmbeddedConstraint::prepareSecondaryToPrimaryMap()
{
  // get mesh pointLocator
  std::unique_ptr<PointLocatorBase> pointLocator = _mesh.getPointLocator();
  pointLocator->enable_out_of_mesh_mode();
  const std::set<subdomain_id_type> allowed_subdomains{_primary};

  // secondary id and primary id
  dof_id_type sid, mid;

  // prepare _secondary_to_primary_map
  std::set<dof_id_type> unique_secondary_node_ids;
  const MeshBase & meshhelper = _mesh.getMesh();
  for (const auto & elem : as_range(meshhelper.active_subdomain_elements_begin(_secondary),
                                    meshhelper.active_subdomain_elements_end(_secondary)))
  {
    for (auto & sn : elem->node_ref_range())
    {
      sid = sn.id();
      if (_secondary_to_primary_map.find(sid) == _secondary_to_primary_map.end())
      {
        // primary element
        const Elem * me = pointLocator->operator()(sn, &allowed_subdomains);
        if (me != NULL)
        {
          mid = me->id();
          _secondary_to_primary_map.insert(std::pair<dof_id_type, dof_id_type>(sid, mid));
          _subproblem.addGhostedElem(mid);
        }
      }
    }
  }
}

bool
EqualValueEmbeddedConstraint::shouldApply()
{
  // primary element
  auto it = _secondary_to_primary_map.find(_current_node->id());

  if (it != _secondary_to_primary_map.end())
  {
    const Elem * primary_elem = _mesh.elemPtr(it->second);
    std::vector<Point> points = {*_current_node};

    // reinit variables on the primary element at the secondary point
    _fe_problem.setNeighborSubdomainID(primary_elem, 0);
    _fe_problem.reinitNeighborPhys(primary_elem, points, 0);

    reinitConstraint();

    return true;
  }
  return false;
}

void
EqualValueEmbeddedConstraint::reinitConstraint()
{
  const Node * node = _current_node;
  unsigned int sys_num = _sys.number();
  dof_id_type dof_number = node->dof_number(sys_num, _var.number(), 0);

  switch (_formulation)
  {
    case Formulation::KINEMATIC:
      _constraint_residual = -_residual_copy(dof_number);
      break;

    case Formulation::PENALTY:
      _constraint_residual = _penalty * (_u_secondary[0] - _u_primary[0]);
      break;

    default:
      mooseError("Invalid formulation");
      break;
  }
}

Real
EqualValueEmbeddedConstraint::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

Real
EqualValueEmbeddedConstraint::computeQpResidual(Moose::ConstraintType type)
{
  Real resid = _constraint_residual;

  switch (type)
  {
    case Moose::Secondary:
    {
      if (_formulation == Formulation::KINEMATIC)
      {
        Real pen_force = _penalty * (_u_secondary[_qp] - _u_primary[_qp]);
        resid += pen_force;
      }
      return _test_secondary[_i][_qp] * resid;
    }

    case Moose::Primary:
      return _test_primary[_i][_qp] * -resid;
  }

  return 0.0;
}

Real
EqualValueEmbeddedConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  unsigned int sys_num = _sys.number();
  const Real penalty = _penalty;
  Real curr_jac, secondary_jac;

  switch (type)
  {
    case Moose::SecondarySecondary:
      switch (_formulation)
      {
        case Formulation::KINEMATIC:
          curr_jac = (*_jacobian)(_current_node->dof_number(sys_num, _var.number(), 0),
                                  _connected_dof_indices[_j]);
          return -curr_jac + _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp];
        case Formulation::PENALTY:
          return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp];
        default:
          mooseError("Invalid formulation");
      }

    case Moose::SecondaryPrimary:
      switch (_formulation)
      {
        case Formulation::KINEMATIC:
          return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp];
        case Formulation::PENALTY:
          return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp];
        default:
          mooseError("Invalid formulation");
      }

    case Moose::PrimarySecondary:
      switch (_formulation)
      {
        case Formulation::KINEMATIC:
          secondary_jac = (*_jacobian)(_current_node->dof_number(sys_num, _var.number(), 0),
                                       _connected_dof_indices[_j]);
          return secondary_jac * _test_primary[_i][_qp];
        case Formulation::PENALTY:
          return -_phi_secondary[_j][_qp] * penalty * _test_primary[_i][_qp];
        default:
          mooseError("Invalid formulation");
      }

    case Moose::PrimaryPrimary:
      switch (_formulation)
      {
        case Formulation::KINEMATIC:
          return 0.0;
        case Formulation::PENALTY:
          return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp];
        default:
          mooseError("Invalid formulation");
      }

    default:
      mooseError("Unsupported type");
      break;
  }
  return 0.0;
}

Real
EqualValueEmbeddedConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                       unsigned int /*jvar*/)
{
  Real curr_jac, secondary_jac;
  unsigned int sys_num = _sys.number();

  switch (type)
  {
    case Moose::SecondarySecondary:
      curr_jac = (*_jacobian)(_current_node->dof_number(sys_num, _var.number(), 0),
                              _connected_dof_indices[_j]);
      return -curr_jac;

    case Moose::SecondaryPrimary:
      return 0.0;

    case Moose::PrimarySecondary:
      switch (_formulation)
      {
        case Formulation::KINEMATIC:
          secondary_jac = (*_jacobian)(_current_node->dof_number(sys_num, _var.number(), 0),
                                       _connected_dof_indices[_j]);
          return secondary_jac * _test_primary[_i][_qp];
        case Formulation::PENALTY:
          return 0.0;
        default:
          mooseError("Invalid formulation");
      }

    case Moose::PrimaryPrimary:
      return 0.0;

    default:
      mooseError("Unsupported type");
      break;
  }

  return 0.0;
}

void
EqualValueEmbeddedConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(
      Moose::NeighborNeighbor, _primary_var.number(), _var.number());

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SecondarySecondary);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());
  if (Ken.m() && Ken.n())
    for (_i = 0; _i < _test_secondary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        Ken(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);

  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());
  for (_i = 0; _i < _test_primary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += computeQpJacobian(Moose::PrimarySecondary);

  if (Knn.m() && Knn.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        Knn(_i, _j) += computeQpJacobian(Moose::PrimaryPrimary);
}

void
EqualValueEmbeddedConstraint::computeOffDiagJacobian(const unsigned int jvar_num)
{
  getConnectedDofIndices(jvar_num);

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _primary_var.number(), jvar_num);

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SecondarySecondary, jvar_num);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar_num);
  for (_i = 0; _i < _test_secondary.size(); _i++)
    for (_j = 0; _j < _phi_primary.size(); _j++)
      Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SecondaryPrimary, jvar_num);

  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());
  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::PrimarySecondary, jvar_num);

  for (_i = 0; _i < _test_primary.size(); _i++)
    for (_j = 0; _j < _phi_primary.size(); _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::PrimaryPrimary, jvar_num);
}

void
EqualValueEmbeddedConstraint::getConnectedDofIndices(unsigned int var_num)
{
  NodeElemConstraint::getConnectedDofIndices(var_num);

  _phi_secondary.resize(_connected_dof_indices.size());

  dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();

  // Fill up _phi_secondary so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  _qp = 0;
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_secondary[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_secondary[j][_qp] = 1.0;
    else
      _phi_secondary[j][_qp] = 0.0;
  }
}
