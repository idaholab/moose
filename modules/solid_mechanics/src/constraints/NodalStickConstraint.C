//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NodalStickConstraint.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SystemBase.h"

// C++ includes
#include <limits.h>

using namespace libMesh;

registerMooseObject("SolidMechanicsApp", NodalStickConstraint);

InputParameters
NodalStickConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addClassDescription("Sticky nodal constraint for contact");
  params.addRequiredParam<BoundaryName>("boundary", "The primary boundary");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<Real>("penalty", "Stiffness of the spring.");
  return params;
}

NodalStickConstraint::NodalStickConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _primary_boundary_id(getParam<BoundaryName>("boundary")),
    _secondary_boundary_id(getParam<BoundaryName>("secondary")),
    _penalty(getParam<Real>("penalty"))
{
  if (_var.number() != _var_secondary.number())
    paramError("variable_secondary",
               "Primary variable must be identical to secondary variable. "
               "Different variables are currently not supported.");

  updateConstrainedNodes();
}

void
NodalStickConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
NodalStickConstraint::updateConstrainedNodes()
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

  const auto elem_ids = gatherAndRetainConnectedElems(_fe_problem.mesh(), _primary_node_vector);
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
}

void
NodalStickConstraint::computeJacobian(const SparseMatrix<Number> & jacobian)
{
  // Calculate Jacobian enteries and cache those entries along with the row and column indices
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
    switch (_formulation)
    {
      case Moose::Penalty:
        Kee(_j, _j) += computeQpJacobian(Moose::PrimaryPrimary);
        Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
        Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
        Knn(_i, _i) += computeQpJacobian(Moose::SecondarySecondary);
        break;
      case Moose::Kinematic:
        Kee(_j, _j) = 0.;
        Ken(_j, _i) += jacobian(secondarydof[_i], primarydof[_j]);
        Kne(_i, _j) += -jacobian(secondarydof[_i], primarydof[_j]) +
                       computeQpJacobian(Moose::SecondaryPrimary);
        Knn(_i, _i) += -jacobian(secondarydof[_i], secondarydof[_i]) +
                       computeQpJacobian(Moose::SecondarySecondary);
        break;
    }
  }
  addJacobian(_assembly, Kee, primarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Knn, secondarydof, secondarydof, _var.scalingFactor());
}

void
NodalStickConstraint::computeResidual(const NumericVector<Number> & residual)
{
  std::vector<dof_id_type> primarydof = _var.dofIndices();
  std::vector<dof_id_type> secondarydof = _var.dofIndicesNeighbor();
  DenseVector<Number> re(primarydof.size());
  DenseVector<Number> neighbor_re(secondarydof.size());

  re.zero();
  neighbor_re.zero();
  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _primary_conn[_i];
    switch (_formulation)
    {
      case Moose::Penalty:
        re(_j) += computeQpResidual(Moose::Primary) * _var.scalingFactor();
        neighbor_re(_i) += computeQpResidual(Moose::Secondary) * _var.scalingFactor();
        break;
      case Moose::Kinematic:
        // Transfer the current residual of the secondary node to the primary nodes
        Real res = residual(secondarydof[_i]);
        re(_j) += res;
        neighbor_re(_i) += -res + computeQpResidual(Moose::Secondary);
        break;
    }
  }
  // We've already applied scaling
  addResiduals(_assembly, re, primarydof, /*scaling_factor=*/1);
  addResiduals(_assembly, neighbor_re, secondarydof, /*scaling_fator=*/1);
}

Real
NodalStickConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Secondary:
      return (_u_secondary[_i] - _u_primary[_j]) * _penalty;
    case Moose::Primary:
      return (_u_primary[_j] - _u_secondary[_i]) * _penalty;
  }
  return 0.;
}

Real
NodalStickConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SecondarySecondary:
      return _penalty;
    case Moose::SecondaryPrimary:
      return -_penalty;
    case Moose::PrimaryPrimary:
      return _penalty;
    case Moose::PrimarySecondary:
      return -_penalty;
    default:
      mooseError("Invalid type");
  }
  return 0.;
}
