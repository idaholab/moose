//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NodalFrictionalConstraint.h"
#include "NodalConstraintUtils.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SystemBase.h"

// C++ includes
#include <limits.h>

using namespace libMesh;

registerMooseObject("SolidMechanicsApp", NodalFrictionalConstraint);

InputParameters
NodalFrictionalConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addClassDescription("Frictional nodal constraint for contact");
  params.addRequiredParam<BoundaryName>("boundary", "The primary boundary");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<Real>("friction_coefficient",
                                "Friction coefficient for slippage in the normal direction");
  params.addRequiredParam<Real>("normal_force",
                                "Normal force used together with friction_coefficient to compute "
                                "the normal frictional capacity.");
  params.addRequiredParam<Real>("tangential_penalty",
                                "Stiffness of the spring in the tangential direction.");
  return params;
}

NodalFrictionalConstraint::NodalFrictionalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _primary_boundary_id(getParam<BoundaryName>("boundary")),
    _secondary_boundary_id(getParam<BoundaryName>("secondary")),
    _normal_force(getParam<Real>("normal_force")),
    _tangential_penalty(getParam<Real>("tangential_penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _u_secondary_old(_var.dofValuesOldNeighbor()),
    _u_primary_old(_var.dofValuesOld())
{
  if (_var.number() != _var_secondary.number())
    paramError("variable_secondary",
               "Primary variable must be identical to secondary "
               "variable. Different variables are currently not supported.");

  updateConstrainedNodes();

  MooseEnum temp_formulation = getParam<MooseEnum>("formulation");
  if (temp_formulation == "penalty")
    _formulation = Moose::Penalty;
  else if (temp_formulation == "kinematic")
    mooseError("NodalFrictionalConstraint: Kinematic formulation is currently not supported for "
               "this constraint.");
  else
    mooseError("Formulation must be set to Penalty.");
}

void
NodalFrictionalConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
NodalFrictionalConstraint::updateConstrainedNodes()
{
  _primary_node_vector.clear();
  _connected_nodes.clear();
  _primary_conn.clear();

  std::vector<dof_id_type> secondary_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_secondary_boundary_id));
  std::vector<dof_id_type> primary_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_primary_boundary_id));

  // Fill in _connected_nodes, which defines secondary nodes in the base class
  for (auto in : secondary_nodelist)
  {
    if (_mesh.nodeRef(in).processor_id() == _subproblem.processor_id())
      _connected_nodes.push_back(in);
  }

  // Fill in _primary_node_vector, which defines secondary nodes in the base class
  for (auto in : primary_nodelist)
    _primary_node_vector.push_back(in);

  const auto elem_ids =
      gatherAndRetainConnectedElems(_fe_problem.mesh(false), _primary_node_vector);
  for (const auto elem_id : elem_ids)
    _subproblem.addGhostedElem(elem_id);

  if (const auto displaced_problem = _fe_problem.getDisplacedProblem())
  {
    const auto displaced_elem_ids =
        gatherAndRetainConnectedElems(displaced_problem->mesh(), _primary_node_vector);
    if (displaced_elem_ids != elem_ids)
      mooseError("Reference and displaced meshes selected different primary elements");
  }

  // Cache map between secondary node and primary node
  _connected_nodes.clear();
  _primary_conn.clear();
  for (unsigned int j = 0; j < secondary_nodelist.size(); ++j)
  {
    if (_mesh.nodeRef(secondary_nodelist[j]).processor_id() == _subproblem.processor_id())
    {
      Node & secondary_node = _mesh.nodeRef(secondary_nodelist[j]);
      for (unsigned int i = 0; i < _primary_node_vector.size(); ++i)
      {
        Node & primary_node = _mesh.nodeRef(_primary_node_vector[i]);
        Real d = (secondary_node - primary_node).norm();
        if (MooseUtils::absoluteFuzzyEqual(d, 0.0))
        {
          _primary_conn.push_back(i);
          _connected_nodes.push_back(secondary_nodelist[j]);
          break;
        }
      }
    }
  }

  _console << "total secondary nodes, primary nodes: " << _primary_conn.size() << ", "
           << _primary_node_vector.size() << '\n';
}

void
NodalFrictionalConstraint::computeResidual(const NumericVector<Number> &
                                           /*residual*/)
{
  const auto & primarydof = _var.dofIndices();
  const auto & secondarydof = _var.dofIndicesNeighbor();
  std::vector<Number> re(primarydof.size());
  std::vector<Number> neighbor_re(secondarydof.size());

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _primary_conn[_i];
    re[_j] += computeQpResidual(Moose::Primary);
    neighbor_re[_i] += computeQpResidual(Moose::Secondary);
    break;
  }
  addResiduals(_assembly, re, primarydof, _var.scalingFactor());
  addResiduals(_assembly, neighbor_re, secondarydof, _var.scalingFactor());
}

Real
NodalFrictionalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  // check whether the tangential spring is already in the yielded state
  Real old_force = (_u_secondary_old[_i] - _u_primary_old[_j]) * _tangential_penalty;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(old_force),
                                           _friction_coefficient * _normal_force))
    old_force = _friction_coefficient * _normal_force * old_force / std::abs(old_force);

  Real current_force =
      ((_u_secondary[_i] - _u_secondary_old[_i]) - (_u_primary[_j] - _u_primary_old[_j])) *
          _tangential_penalty +
      old_force;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(current_force),
                                           _friction_coefficient * _normal_force))
    current_force = _friction_coefficient * _normal_force * current_force / std::abs(current_force);

  switch (type)
  {
    case Moose::Secondary:
      return current_force;
    case Moose::Primary:
      return -current_force;
  }
  return 0;
}

void
NodalFrictionalConstraint::computeJacobian(const SparseMatrix<Number> & /*jacobian*/)
{
  // Calculate Jacobian entries and cache those entries along with the row and column indices
  std::vector<dof_id_type> secondarydof = _var.dofIndicesNeighbor();
  std::vector<dof_id_type> primarydof = _var.dofIndices();

  DenseMatrix<Number> Kee(primarydof.size(), primarydof.size());
  DenseMatrix<Number> Ken(primarydof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), primarydof.size());
  DenseMatrix<Number> Knn(secondarydof.size(), secondarydof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();
  Knn.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _primary_conn[_i];
    Kee(_j, _j) += computeQpJacobian(Moose::PrimaryPrimary);
    Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
    Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
    Knn(_i, _i) += computeQpJacobian(Moose::SecondarySecondary);
  }
  addJacobian(_assembly, Kee, primarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Knn, secondarydof, secondarydof, _var.scalingFactor());
}

Real
NodalFrictionalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  Real jac = _tangential_penalty;

  // set jacobian to zero if spring has yielded
  Real old_force = (_u_secondary_old[_i] - _u_primary_old[_j]) * _tangential_penalty;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(old_force),
                                           _friction_coefficient * _normal_force))
    old_force = _friction_coefficient * _normal_force * old_force / std::abs(old_force);

  Real current_force =
      ((_u_secondary[_i] - _u_secondary_old[_i]) - (_u_primary[_j] - _u_primary_old[_j])) *
          _tangential_penalty +
      old_force;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(current_force),
                                           _friction_coefficient * _normal_force))
    jac = 0.0;

  switch (type)
  {
    case Moose::SecondarySecondary:
      return jac;
    case Moose::SecondaryPrimary:
      return -jac;
    case Moose::PrimaryPrimary:
      return jac;
    case Moose::PrimarySecondary:
      return -jac;
    default:
      mooseError("Invalid type");
  }
  return 0.;
}
