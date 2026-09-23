//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EigenADReal.h"
#include "EqualValueEmbeddedConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "AuxiliarySystem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "AddVariableAction.h"

#include "libmesh/dof_map.h"
#include "libmesh/fe_interface.h"
#include "libmesh/fe_map.h"
#include "libmesh/sparse_matrix.h"

#include <algorithm>

registerMooseObject("MooseApp", EqualValueEmbeddedConstraint);
registerMooseObject("MooseApp", ADEqualValueEmbeddedConstraint);

template <bool is_ad>
InputParameters
EqualValueEmbeddedConstraintTempl<is_ad>::validParams()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  InputParameters params = GenericNodeElemConstraint<is_ad>::validParams();
  params.addClassDescription("This is a constraint enforcing overlapping portions of two blocks to "
                             "have the same variable value");
  params.set<bool>("use_displaced_mesh") = false;
  MooseEnum formulation(getFormulationOptions(), "kinematic");
  params.addParam<MooseEnum>("formulation",
                             formulation,
                             "Formulation used to enforce the constraint. With 'rows' the "
                             "constraint assembles no residual and no Jacobian: it hands one "
                             "degree of freedom constraint row per secondary node to the DofMap, "
                             "which enforces the constraint exactly and needs no penalty.");
  params.addParam<Real>(
      "penalty",
      "Penalty parameter used in constraint enforcement for kinematic and penalty formulations. "
      "It is required with those two formulations and unused with the rows formulation.");

  return params;
}

template <bool is_ad>
EqualValueEmbeddedConstraintTempl<is_ad>::EqualValueEmbeddedConstraintTempl(
    const InputParameters & parameters)
  : GenericNodeElemConstraint<is_ad>(parameters),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _formulation(this->template getParam<MooseEnum>("formulation").template getEnum<Formulation>()),
    _penalty(this->isParamValid("penalty") ? this->template getParam<Real>("penalty") : 0.0),
    _residual_copy(_sys.residualGhosted())
{
  _overwrite_secondary_residual = false;

  if (_formulation != Formulation::ROWS && !this->isParamValid("penalty"))
    this->paramError("penalty",
                     "A penalty is required with formulation '",
                     this->template getParam<MooseEnum>("formulation"),
                     "'. It may only be omitted with formulation 'rows', which enforces the "
                     "constraint with degree of freedom constraint rows instead of a penalty "
                     "term.");

  prepareSecondaryToPrimaryMap();

  if constexpr (is_ad)
  {
    if (_formulation == Formulation::KINEMATIC)
      this->paramError("formulation", "AD constraints cannot be used with KINEMATIC formulation.");
  }
}

template <bool is_ad>
void
EqualValueEmbeddedConstraintTempl<is_ad>::prepareSecondaryToPrimaryMap()
{
  // get mesh pointLocator
  std::unique_ptr<libMesh::PointLocatorBase> pointLocator = _mesh.getPointLocator();
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

template <bool is_ad>
void
EqualValueEmbeddedConstraintTempl<is_ad>::addConstraintRows(libMesh::DofMap & dof_map) const
{
  const auto sys_num = _sys.number();
  const MooseVariable & primary_var = this->_primary_var;
  const libMesh::FEType & fe_type = primary_var.feType();

  // A secondary node a nodal boundary condition already pins keeps its boundary condition, which
  // is what the penalty and kinematic formulations do
  const auto pinned_nodes = this->nodesPinnedByNodalBCs(_var.name());

  // This method is collective: libMesh calls it on every rank each time it rebuilds the
  // constraints of the system
  Constraint::CollectiveError error(*this);

  std::vector<dof_id_type> primary_dofs;
  for (const auto & [secondary_id, primary_id] : _secondary_to_primary_map)
  {
    // The ranks that have both the secondary node and its primary element add the row, and they
    // all add the same one. A rank that has neither has nothing to constrain
    const Node * const secondary_node = _mesh.queryNodePtr(secondary_id);
    const Elem * const primary_elem = _mesh.queryElemPtr(primary_id);
    if (!secondary_node || !primary_elem)
      continue;

    // A nodal boundary condition already pins this dof, and it wins, as it does under the penalty
    // and kinematic formulations
    if (pinned_nodes.count(secondary_id))
      continue;

    if (secondary_node->n_comp(sys_num, _var.number()) == 0)
    {
      error.record("The variable '" + _var.name() +
                   "' has no degree of freedom at the secondary node " +
                   std::to_string(secondary_id) + ", so this constraint cannot tie it there.");
      break;
    }
    const auto secondary_dof = secondary_node->dof_number(sys_num, _var.number(), 0);

    dof_map.dof_indices(primary_elem, primary_dofs, primary_var.number());
    if (primary_dofs.empty())
    {
      error.record("The variable '" + primary_var.name() +
                   "' has no degrees of freedom on the primary element " +
                   std::to_string(primary_id) + ", which contains the secondary node " +
                   std::to_string(secondary_id) +
                   ", so this constraint has nothing to tie that node to.");
      break;
    }

    // A node the two blocks share constrains its own degree of freedom. The tie is satisfied there
    // by itself, and a row that makes a degree of freedom depend on itself is not a constraint
    if (std::find(primary_dofs.begin(), primary_dofs.end(), secondary_dof) != primary_dofs.end())
      continue;

    // The reference coordinate of the secondary node in the primary element, the same point the
    // residual path reinitializes the primary variable at through
    // Assembly::reinitNeighborAtPhysical()
    const Point reference_point =
        libMesh::FEMap::inverse_map(primary_elem->dim(), primary_elem, *secondary_node);

    // FEInterface::shape() numbers the shape functions of an element the way DofMap::dof_indices()
    // numbers its degrees of freedom, so weight i belongs to primary_dofs[i]
    libMesh::DofConstraintRow row;
    for (const auto i : index_range(primary_dofs))
      row[primary_dofs[i]] = libMesh::FEInterface::shape(fe_type, primary_elem, i, reference_point);

    // Forbid overwriting so that a secondary dof libMesh already constrains, a hanging node for
    // instance, errors out instead of silently losing one of the two constraints
    dof_map.add_constraint_row(secondary_dof, row, /*forbid_constraint_overwrite=*/true);
  }

  error.raise();
}

template <bool is_ad>
bool
EqualValueEmbeddedConstraintTempl<is_ad>::shouldApply()
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

template <bool is_ad>
void
EqualValueEmbeddedConstraintTempl<is_ad>::reinitConstraint()
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

template <bool is_ad>
Real
EqualValueEmbeddedConstraintTempl<is_ad>::computeQpSecondaryValue()
{
  return MetaPhysicL::raw_value(_u_secondary[_qp]);
}

template <bool is_ad>
GenericReal<is_ad>
EqualValueEmbeddedConstraintTempl<is_ad>::computeQpResidual(Moose::ConstraintType type)
{
  GenericReal<is_ad> resid = _constraint_residual;

  switch (type)
  {
    case Moose::Secondary:
    {
      if (_formulation == Formulation::KINEMATIC)
      {
        GenericReal<is_ad> pen_force = _penalty * (_u_secondary[_qp] - _u_primary[_qp]);
        resid += pen_force;
      }
      return _test_secondary[_i][_qp] * resid;
    }

    case Moose::Primary:
      return _test_primary[_i][_qp] * -resid;
  }

  return 0.0;
}

template <bool is_ad>
Real
EqualValueEmbeddedConstraintTempl<is_ad>::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  mooseAssert(!is_ad,
              "In ADEqualValueEmbeddedConstraint, computeQpJacobian should not be called. "
              "Check computeJacobian implementation.");

  unsigned int sys_num = _sys.number();
  const Real penalty = MetaPhysicL::raw_value(_penalty);
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

template <bool is_ad>
Real
EqualValueEmbeddedConstraintTempl<is_ad>::computeQpOffDiagJacobian(
    Moose::ConstraintJacobianType type, unsigned int /*jvar*/)
{
  mooseAssert(!is_ad,
              "In ADEqualValueEmbeddedConstraint, computeQpOffDiagJacobian should not be called. "
              "Check computeJacobian implementation.");

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

template class EqualValueEmbeddedConstraintTempl<false>;
template class EqualValueEmbeddedConstraintTempl<true>;
