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
RANFSNormalMechanicalContact::timestepSetup()
{
  _node_to_master_elem_sequence.clear();
  _ping_pong_slave_node_to_master_node.clear();
}

void
RANFSNormalMechanicalContact::residualSetup()
{
  _node_to_lm.clear();
  NodeFaceConstraint::residualSetup();
}

bool
RANFSNormalMechanicalContact::overwriteSlaveResidual()
{
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

      // Check to see whether we've locked a ping-ponging node
      if (_ping_pong_slave_node_to_master_node.find(_current_node->id()) ==
          _ping_pong_slave_node_to_master_node.end())
      {
        if (_lagrange_multiplier > -_pinfo->_distance)
        {
          // Ok, our math is telling us we should apply the constraint, but what if we are
          // ping-ponging back and forth between different master faces? If we are then let's try to
          // not apply the constraint

          // This only works for a basic line search! Write assertion here
          if (_subproblem.computingNonlinearResid())
          {
            auto & master_elem_sequence = _node_to_master_elem_sequence[_current_node->id()];
            mooseAssert(
                _current_master == _pinfo->_elem,
                "The current master element and the PenetrationInfo object's element should "
                "be the same");
            master_elem_sequence.push_back(_pinfo->_elem);

            if (master_elem_sequence.size() >= 5 &&
                _pinfo->_elem == *(master_elem_sequence.rbegin() + 2) &&
                _pinfo->_elem == *(master_elem_sequence.rbegin() + 4) &&
                _pinfo->_elem != *(master_elem_sequence.rbegin() + 1) &&
                *(master_elem_sequence.rbegin() + 1) == *(master_elem_sequence.rbegin() + 3))
            {
              // Ok we are ping-ponging

              // Let's figure out the master node that we should use for
              // determining distance
              Real max_phi = 0;
              unsigned int master_node_local_index = 0;
              for (MooseIndex(_test_master) i = 0; i < _test_master.size(); ++i)
              {
                mooseAssert(_test_master[i].size() == 1,
                            "There should only be one quadrature point that we project onto");
                Real phi = _test_master[i][0];
                if (phi > max_phi)
                {
                  max_phi = phi;
                  master_node_local_index = i;
                }
              }
              auto master_node = _pinfo->_elem->node_ptr(master_node_local_index);

              // Ok now let's find the neighboring element that also shares this node
              const Elem * neighbor_elem = nullptr;
              unsigned int master_node_neighbor_index;
              for (auto current_neighbor : _pinfo->_elem->neighbor_ptr_range())
              {
                if (!current_neighbor)
                  continue;

                master_node_neighbor_index = current_neighbor->get_node_index(master_node);
                if (master_node_neighbor_index != libMesh::invalid_uint)
                {
                  neighbor_elem = current_neighbor;
                  break;
                }
              }
              mooseAssert(neighbor_elem, "We didn't find a neighboring element!");

              // And what side of the neighbor has this node?
              unsigned int neighbor_side = libMesh::invalid_uint;
              for (unsigned int side = 0; side < neighbor_elem->n_sides(); ++side)
                if (!neighbor_elem->neighbor_ptr(side) &&
                    neighbor_elem->is_node_on_side(master_node_neighbor_index, side))
                  neighbor_side = side;
              mooseAssert(neighbor_side != libMesh::invalid_uint,
                          "We were unable to find the side that the node lives on!");

              _ping_pong_slave_node_to_master_node.insert(
                  std::make_pair<dof_id_type, MasterNodeInfo>(
                      _current_node->id(),
                      {master_node,
                       std::make_pair(std::make_pair(_pinfo->_elem, _pinfo->_side_num),
                                      std::make_pair(neighbor_elem, neighbor_side))}));
            }
          }
        }
        else
          // We have not locked the node into contact nor is the gap smaller than the Lagrange
          // Multiplier so we should not apply
          return false;
      }

      auto it = _ping_pong_slave_node_to_master_node.find(_current_node->id());
      if (it != _ping_pong_slave_node_to_master_node.end())
      {
        // We need to compute master nodal normal

        auto & master_node_ref = *it->second.master_node;

        auto & master_node_info = it->second;
        auto & master1 = master_node_info.master_elems_and_sides.first;
        auto & master2 = master_node_info.master_elems_and_sides.second;

        auto side_elem1 = master1.first->build_side_ptr(master1.second);
        auto side_elem2 = master2.first->build_side_ptr(master2.second);

        // Determine the correct reference coordinate
        FEMap map1, map2;
        auto ref1 = map1.inverse_map(/*dim=*/1, side_elem1.get(), master_node_ref);
        auto ref2 = map2.inverse_map(/*dim=*/1, side_elem2.get(), master_node_ref);

        // Now calculate the normals
        auto & normals1 = map1.get_normals();
        auto & normals2 = map2.get_normals();

        // We apparently also need to do some inverse mapping that requires xyz to exist
        map1.get_xyz();
        map2.get_xyz();

        map1.init_face_shape_functions</*Dim=*/2>({ref1}, side_elem1.get());
        map2.init_face_shape_functions</*Dim=*/2>({ref2}, side_elem2.get());

        map1.compute_face_map(/*dim=*/2, /*dummy_qw=*/{1}, side_elem1.get());
        map2.compute_face_map(/*dim=*/2, /*dummy_qw=*/{1}, side_elem2.get());

        mooseAssert(normals1.size() == 1, "There should only have been one reference point");
        mooseAssert(normals2.size() == 1, "There should only have been one reference point");

        auto master_nodal_normal = (normals1[0] + normals2[0]) / 2.;

        auto distance_vec = *_current_node - master_node_ref;
        _distance = distance_vec.norm();
        _normal_component = master_nodal_normal(_component);
        _restrict_master_residual = true;

        auto _master_index = _current_master->get_node_index(&master_node_ref);
        mooseAssert(_master_index != libMesh::invalid_uint,
                    "The master node does not exist on the current master element");
      }
      else
      {
        _distance = _pinfo->_distance;
        _normal_component = _pinfo->_normal(_component);
        // Do this to make sure constraint equation has a positive on the diagonal
        if (_normal_component > 0)
          _distance *= -1;
        _restrict_master_residual = false;
      }

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
      if (_largest_component == static_cast<unsigned int>(_component))
      {
        mooseAssert(_normal_component != 0,
                    "We should be selecting the largest normal component, hence it should be "
                    "impossible for this normal component to be zero");

        return _distance;
      }

      else
        // The normal points out of the master face
        return _lagrange_multiplier * -_normal_component;
    }

    case Moose::ConstraintType::Master:
    {
      if (_restrict_master_residual)
      {
        if (_i == _master_index)
          return _lagrange_multiplier * _normal_component;
        else
          return 0;
      }
      else
        return _test_master[_i][_qp] * _lagrange_multiplier * _normal_component;

      default:
        return 0;
    }
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
        // _phi_slave has been set such that it is 1 when _j corresponds to the degree of freedom
        // associated with the _current node and 0 otherwise
        return std::abs(_normal_component) * _phi_slave[_j][_qp];

      else
        return 0;
    }

    case Moose::ConstraintJacobianType::SlaveMaster:
    {
      if (_largest_component == static_cast<unsigned int>(_component))
        return -std::abs(_normal_component) * _phi_master[_j][_qp];

      else
        return 0;
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
