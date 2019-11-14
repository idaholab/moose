//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RANFSTieNode.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/petsc_matrix.h"

registerMooseObject("ContactApp", RANFSTieNode);

template <>
InputParameters
validParams<RANFSTieNode>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  return params;
}

RANFSTieNode::RANFSTieNode(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _mesh_dimension(_mesh.dimension()),
    _residual_copy(_sys.residualGhosted())
{
  // modern parameter scheme for displacements
  for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
  {
    _vars.push_back(coupled("displacements", i));
    _var_objects.push_back(getVar("displacements", i));
  }

  if (_vars.size() != _mesh_dimension)
    mooseError("The number of displacement variables does not match the mesh dimension!");
}

void
RANFSTieNode::initialSetup()
{
}

void
RANFSTieNode::timestepSetup()
{
}

void
RANFSTieNode::residualSetup()
{
  _node_to_lm.clear();
}

void
RANFSTieNode::jacobianSetup()
{
  _jacobian = &_sys.getMatrix(_sys.systemMatrixTag());
}

bool
RANFSTieNode::overwriteSlaveResidual()
{
  return _nearest_node;
}

bool
RANFSTieNode::overwriteSlaveJacobian()
{
  // We did it ourselves
  return false;
}

bool
RANFSTieNode::shouldApply()
{
  auto & nearest_node_loc = _penetration_locator._nearest_node;
  _nearest_node = nearest_node_loc.nearestNode(_current_node->id());
  if (_nearest_node)
  {
    _dof_number = static_cast<PetscInt>(_current_node->dof_number(0, _vars[_component], 0));
    // We overwrite the slave residual so we cannot use the residual
    // copy for determining the Lagrange multiplier when computing the Jacobian
    if (!_subproblem.currentlyComputingJacobian())
      _node_to_lm.insert(std::make_pair(_current_node->id(),
                                        _residual_copy(static_cast<dof_id_type>(_dof_number)) /
                                            _var_objects[_component]->scalingFactor()));
    else
    {
      // Let's get the Jacobian row coresponding to the slave node
      auto petsc_mat = dynamic_cast<PetscMatrix<Number> *>(_jacobian);
      if (!petsc_mat)
        mooseError("This only works with a Petsc matrix");

      // We need the matrix to be assembled so we get the correct Jacobian entries
      if (!petsc_mat->closed())
        petsc_mat->close();

      _mat = petsc_mat->mat();

      const PetscInt * master_cols;
      const PetscScalar * master_values;
      PetscInt master_ncols;
      PetscErrorCode ierr =
          MatGetRow(_mat, _dof_number, &master_ncols, &master_cols, &master_values);
      LIBMESH_CHKERR(ierr);

      // Copy the data
      _master_ncols = master_ncols;
      _master_cols.assign(master_cols, master_cols + _master_ncols);
      _master_values.assign(master_values, master_values + _master_ncols);

      // Now restore
      ierr = MatRestoreRow(_mat, _dof_number, &master_ncols, &master_cols, &master_values);
      LIBMESH_CHKERR(ierr);
    }

    mooseAssert(_node_to_lm.find(_current_node->id()) != _node_to_lm.end(),
                "The node " << _current_node->id() << " should map to a lagrange multiplier");
    _lagrange_multiplier = _node_to_lm[_current_node->id()];

    _master_index = _current_master->get_node_index(_nearest_node);
    mooseAssert(_master_index != libMesh::invalid_uint,
                "nearest node not a node on the current master element");

    _master_dof_number = static_cast<PetscInt>(_nearest_node->dof_number(0, _vars[_component], 0));

    return true;
  }

  return false;
}

Real
RANFSTieNode::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Slave:
      return (*_current_node - *_nearest_node)(_component);

    case Moose::ConstraintType::Master:
    {
      if (_i == _master_index)
        return _lagrange_multiplier;

      else
        return 0;
    }

    default:
      return 0;
  }
}

Real RANFSTieNode::computeQpJacobian(Moose::ConstraintJacobianType)
{
  mooseError("This shouldn't get called");
}

void
RANFSTieNode::computeSlaveValue(NumericVector<Number> &)
{
}

Real
RANFSTieNode::computeQpSlaveValue()
{
  mooseError("We overrode commputeSlaveValue so computeQpSlaveValue should never get called");
}

void
RANFSTieNode::computeJacobian()
{
  // Now set the slave row
  std::vector<PetscInt> slave_row = {_dof_number};
  std::vector<PetscInt> slave_cols = {_dof_number, _master_dof_number};
  std::vector<PetscScalar> slave_values = {1, -1};

  // This is currently a bad design because we're going to (possibly) assemble here and then put the
  // matrix in an unassembled state again

  PetscBool assembled;
  PetscErrorCode ierr = MatAssembled(_mat, &assembled);
  LIBMESH_CHKERR(ierr);

  if (!assembled)
  {
    ierr = MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
    LIBMESH_CHKERR(ierr);
    ierr = MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
    LIBMESH_CHKERR(ierr);
  }
  // This operation requires that the matrix be assembled
  ierr = MatZeroRows(_mat, 1, slave_row.data(), 0, NULL, NULL);
  LIBMESH_CHKERR(ierr);

  ierr = MatSetValues(
      _mat, 1, slave_row.data(), 2, slave_cols.data(), slave_values.data(), ADD_VALUES);
  LIBMESH_CHKERR(ierr);

  std::vector<PetscInt> master_row = {_master_dof_number};
  ierr = MatSetValues(_mat,
                      1,
                      master_row.data(),
                      _master_ncols,
                      _master_cols.data(),
                      _master_values.data(),
                      ADD_VALUES);
  LIBMESH_CHKERR(ierr);
}

void
RANFSTieNode::computeOffDiagJacobian(unsigned int)
{
}
