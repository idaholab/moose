//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RANFSNormalMechanicalContact.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("ContactApp", RANFSNormalMechanicalContact);

template <>
InputParameters
validParams<RANFSNormalMechanicalContact>()
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

RANFSNormalMechanicalContact::RANFSNormalMechanicalContact(const InputParameters & parameters)
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
RANFSNormalMechanicalContact::residualSetup()
{
  _node_to_lm.clear();
  NodeFaceConstraint::residualSetup();
}

bool
RANFSNormalMechanicalContact::shouldApply()
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    _pinfo = found->second;
    if (_pinfo)
    {
      // We overwrite the slave residual when constraints are active so we cannot use the residual
      // copy for determining the Lagrange multiplier when computing the Jacobian
      if (!_subproblem.currentlyComputingJacobian())
      {
        // Build up residual vector corresponding to contact forces
        RealVectorValue res_vec;
        for (unsigned int i = 0; i < _mesh_dimension; ++i)
        {
          dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
          res_vec(i) = _residual_copy(dof_number) / _var_objects[i]->scalingFactor();
        }

        _node_to_lm.insert(std::make_pair(_current_node->id(), res_vec * _pinfo->_normal));
      }

      mooseAssert(_node_to_lm.find(_current_node->id()) != _node_to_lm.end(),
                  "The node " << _current_node->id() << " should map to a lagrange multiplier");
      _lagrange_multiplier = _node_to_lm[_current_node->id()];
      if (_lagrange_multiplier > -_pinfo->_distance)
      {
        // The constraint is active -> we're going to use our linear solve to ensure that the gap
        // is driven to zero. We only have one zero-penetration constraint per node, so we choose
        // to apply the zero penetration constraint only to the displacement component with the
        // largest magnitude normal
        auto largest_component_magnitude = std::abs(_pinfo->_normal(0));
        _largest_component = 0;
        for (MooseIndex(_mesh_dimension) i = 1; i < _mesh_dimension; ++i)
        {
          auto component_magnitude = std::abs(_pinfo->_normal(i));
          if (component_magnitude > largest_component_magnitude)
          {
            largest_component_magnitude = component_magnitude;
            _largest_component = i;
          }
        }

        return true;
      }
    }
  }

  return false;
}

Real
RANFSNormalMechanicalContact::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Slave:
    {
      if (_largest_component == static_cast<unsigned int>(_component))
      {
        _overwrite_slave_residual = true;
        mooseAssert(_pinfo->_normal(_component) != 0,
                    "We should be selecting the largest normal component, hence it should be "
                    "impossible for this normal component to be zero");
        // Do this if-else to make sure that the on-diagonal is positive
        if (_pinfo->_normal(_component) > 0)
          return -_pinfo->_distance;
        else
          return _pinfo->_distance;
      }

      else
      {
        _overwrite_slave_residual = false;
        // The normal points out of the master face
        return _lagrange_multiplier * -_pinfo->_normal(_component);
      }
    }

    case Moose::ConstraintType::Master:
      return _test_master[_i][_qp] * _lagrange_multiplier * _pinfo->_normal(_component);

    default:
      return 0;
  }
}

Real
RANFSNormalMechanicalContact::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::ConstraintJacobianType::SlaveSlave:
    {
      if (_largest_component == static_cast<unsigned int>(_component))
      {
        _overwrite_slave_residual = true;
        return std::abs(_pinfo->_normal(_component));
      }
      else
      {
        _overwrite_slave_residual = false;
        return 0;
      }
    }
    case Moose::ConstraintJacobianType::MasterSlave:
    {
      auto slave_dof_number =
          _current_node->dof_number(/*system_number=*/0, _var.number(), /*variable_component=*/0);

      // At least this matrix entry should be ghosted
      auto slave_on_diagonal_jacobian =
          (*_jacobian)(slave_dof_number, slave_dof_number) / _var.scalingFactor();
      return _test_master[_i][_qp] * _pinfo->_normal(_component) * _pinfo->_normal(_component) *
             slave_on_diagonal_jacobian;
    }

    default:
      return 0;
  }
}

Real
RANFSNormalMechanicalContact::computeQpSlaveValue()
{
  mooseError("Why are you calling me?");
}
