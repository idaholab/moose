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
#include "NearestNodeLocator.h"

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
RANFSNormalMechanicalContact::timestepSetup()
{
  _node_to_master_elem_sequence.clear();
  _ping_pong_slave_node_to_master_node.clear();
}

void
RANFSNormalMechanicalContact::residualSetup()
{
  _node_to_contact_lm.clear();
  _node_to_tied_lm.clear();
  NodeFaceConstraint::residualSetup();
}

bool
RANFSNormalMechanicalContact::overwriteSlaveResidual()
{
  if (_tie_nodes)
    return true;
  else
    return _largest_component == static_cast<unsigned int>(_component);
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
        _res_vec.zero();
        for (unsigned int i = 0; i < _mesh_dimension; ++i)
        {
          dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
          _res_vec(i) = _residual_copy(dof_number) / _var_objects[i]->scalingFactor();
        }

        _node_to_contact_lm.insert(std::make_pair(_current_node->id(), _res_vec * _pinfo->_normal));
        _node_to_tied_lm.insert(std::make_pair(_current_node->id(), _res_vec(_component)));
      }

      mooseAssert(_node_to_contact_lm.find(_current_node->id()) != _node_to_contact_lm.end(),
                  "The node " << _current_node->id()
                              << " should map to a contact lagrange multiplier");
      mooseAssert(_node_to_tied_lm.find(_current_node->id()) != _node_to_tied_lm.end(),
                  "The node " << _current_node->id()
                              << " should map to a tied lagrange multiplier");
      _contact_lm = _node_to_contact_lm[_current_node->id()];
      _tied_lm = _node_to_tied_lm[_current_node->id()];

      // Check to see whether we've locked a ping-ponging node
      if (_ping_pong_slave_node_to_master_node.find(_current_node->id()) ==
          _ping_pong_slave_node_to_master_node.end())
      {
        if (_contact_lm > -_pinfo->_distance)
        {
          // Ok, our math is telling us we should apply the constraint, but what if we are
          // ping-ponging back and forth between different master faces?

          // This only works for a basic line search! Write assertion here
          if (_subproblem.computingNonlinearResid())
          {
            auto & master_elem_sequence = _node_to_master_elem_sequence[_current_node->id()];
            mooseAssert(
                _current_master == _pinfo->_elem,
                "The current master element and the PenetrationInfo object's element should "
                "be the same");
            master_elem_sequence.push_back(_pinfo->_elem);

            // 5 here is a heuristic choice. In testing, generally speaking, if a node ping-pongs
            // back and forth 5 times then it will ping pong indefinitely. However, if it goes 3
            // times for example, it is not guaranteed to ping-pong indefinitely and the Newton
            // iteration may naturally resolve the correct face dofs that need to be involved in the
            // constraint.
            if (master_elem_sequence.size() >= 5 &&
                _pinfo->_elem == *(master_elem_sequence.rbegin() + 2) &&
                _pinfo->_elem == *(master_elem_sequence.rbegin() + 4) &&
                _pinfo->_elem != *(master_elem_sequence.rbegin() + 1) &&
                *(master_elem_sequence.rbegin() + 1) == *(master_elem_sequence.rbegin() + 3))
            {
              // Ok we are ping-ponging. Determine the master node
              auto master_node =
                  _penetration_locator._nearest_node.nearestNode(_current_node->id());

              // Sanity checks
              mooseAssert(_pinfo->_elem->get_node_index(master_node) != libMesh::invalid_uint,
                          "The master node is not on the current element");
              mooseAssert((*(master_elem_sequence.rbegin() + 1))->get_node_index(master_node) !=
                              libMesh::invalid_uint,
                          "The master node is not on the other ping-ponging element");

              _ping_pong_slave_node_to_master_node.insert(
                  std::make_pair(_current_node->id(), master_node));
            }
          }
        }
        else
          // We have not locked the node into contact nor is the gap smaller than the Lagrange
          // Multiplier so we should not apply
          return false;
      }

      // Determine whether we're going to apply the tied node equality constraint or the contact
      // inequality constraint
      auto it = _ping_pong_slave_node_to_master_node.find(_current_node->id());
      if (it != _ping_pong_slave_node_to_master_node.end())
      {
        _tie_nodes = true;
        _nearest_node = it->second;
        _master_index = _current_master->get_node_index(_nearest_node);
        mooseAssert(_master_index != libMesh::invalid_uint,
                    "nearest node not a node on the current master element");
      }
      else
      {
        _distance = _pinfo->_distance;
        // Do this to make sure constraint equation has a positive on the diagonal
        if (_pinfo->_normal(_component) > 0)
          _distance *= -1;
        _tie_nodes = false;

        // The contact constraint is active -> we're going to use our linear solve to ensure that
        // the gap is driven to zero. We only have one zero-penetration constraint per node, so we
        // choose to apply the zero penetration constraint only to the displacement component with
        // the largest magnitude normal
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
      }

      return true;
    }
  }

  // If we're not applying the constraint then we can clear the node to master elem sequence for
  // this node
  if (_node_to_master_elem_sequence.find(_current_node->id()) !=
      _node_to_master_elem_sequence.end())
    _node_to_master_elem_sequence[_current_node->id()].clear();
  return false;
}

Real
RANFSNormalMechanicalContact::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Slave:
    {
      if (_tie_nodes)
        return (*_current_node - *_nearest_node)(_component);
      else
      {
        if (_largest_component == static_cast<unsigned int>(_component))
        {
          mooseAssert(_pinfo->_normal(_component) != 0,
                      "We should be selecting the largest normal component, hence it should be "
                      "impossible for this normal component to be zero");

          return _distance;
        }

        else
          // The normal points out of the master face
          return _contact_lm * -_pinfo->_normal(_component);
      }
    }

    case Moose::ConstraintType::Master:
    {
      if (_tie_nodes)
      {
        if (_i == _master_index)
          return _tied_lm;
        else
          return 0;
      }
      else
        return _test_master[_i][_qp] * _contact_lm * _pinfo->_normal(_component);
    }

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
      if (_tie_nodes)
        return 0;
      else
      {
        if (_largest_component == static_cast<unsigned int>(_component))
          // _phi_slave has been set such that it is 1 when _j corresponds to the degree of freedom
          // associated with the _current node and 0 otherwise
          return std::abs(_pinfo->_normal(_component)) * _phi_slave[_j][_qp];

        else
          return 0;
      }
    }

    case Moose::ConstraintJacobianType::SlaveMaster:
    {
      if (_tie_nodes)
        return 0;
      else
      {
        if (_largest_component == static_cast<unsigned int>(_component))
          return -std::abs(_pinfo->_normal(_component)) * _phi_master[_j][_qp];

        else
          return 0;
      }
    }

    default:
      return 0;
  }
}

void
RANFSNormalMechanicalContact::computeSlaveValue(NumericVector<Number> &)
{
}

Real
RANFSNormalMechanicalContact::computeQpSlaveValue()
{
  mooseError("We overrode commputeSlaveValue so computeQpSlaveValue should never get called");
}
