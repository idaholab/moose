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
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/enum_fe_family.h"

registerMooseObject("ContactApp", RANFSNormalMechanicalContact);

InputParameters
RANFSNormalMechanicalContact::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.set<bool>("use_displaced_mesh") = true;

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<bool>("ping_pong_protection",
                        false,
                        "Whether to protect against ping-ponging, e.g. the oscillation of the "
                        "secondary node between two "
                        "different primary faces, by tying the secondary node to the "
                        "edge between the involved primary faces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addClassDescription("Applies the Reduced Active Nonlinear Function Set scheme in which "
                             "the secondary node's non-linear residual function is replaced by the "
                             "zero penetration constraint equation when the constraint is active");
  return params;
}

RANFSNormalMechanicalContact::RANFSNormalMechanicalContact(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _mesh_dimension(_mesh.dimension()),
    _residual_copy(_sys.residualGhosted()),
    _dof_number_to_value(coupledComponents("displacements")),
    _disp_coupling(coupledComponents("displacements")),
    _ping_pong_protection(getParam<bool>("ping_pong_protection"))
{
  // modern parameter scheme for displacements
  for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
  {
    _vars.push_back(coupled("displacements", i));
    _var_objects.push_back(getVar("displacements", i));
  }

  for (auto & var : _var_objects)
    if (var->feType().family != LAGRANGE)
      mooseError("This object only works when the displacement variables use a Lagrange basis");

  if (_vars.size() != _mesh_dimension)
    mooseError("The number of displacement variables does not match the mesh dimension!");

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
}

void
RANFSNormalMechanicalContact::initialSetup()
{
  auto system_coupling_matrix = _subproblem.couplingMatrix();

  for (MooseIndex(_vars) i = 0; i < _vars.size(); ++i)
    for (MooseIndex(_vars) j = 0; j < _vars.size(); ++j)
      _disp_coupling(i, j) = (*system_coupling_matrix)(_vars[i], _vars[j]);
}

void
RANFSNormalMechanicalContact::timestepSetup()
{
  _node_to_primary_elem_sequence.clear();
  _ping_pong_secondary_node_to_primary_node.clear();
}

void
RANFSNormalMechanicalContact::residualSetup()
{
  _node_to_contact_lm.clear();
  _node_to_tied_lm.clear();
  NodeFaceConstraint::residualSetup();
}

bool
RANFSNormalMechanicalContact::overwriteSecondaryResidual()
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
      // We overwrite the secondary residual when constraints are active so we cannot use the
      // residual copy for determining the Lagrange multiplier when computing the Jacobian
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
      else
      {
        std::vector<dof_id_type> cols;
        std::vector<Number> values;

        for (auto & d_to_v : _dof_number_to_value)
          d_to_v.clear();

        mooseAssert(_vars.size() == _dof_number_to_value.size() &&
                        _vars.size() == _var_objects.size(),
                    "Somehow the sizes of our variable containers got out of sync");
        for (MooseIndex(_var_objects) i = 0; i < _var_objects.size(); ++i)
        {
          auto secondary_dof_number = _current_node->dof_number(0, _vars[i], 0);

          _jacobian->get_row(secondary_dof_number, cols, values);
          mooseAssert(cols.size() == values.size(),
                      "The size of the dof container and value container are different");

          for (MooseIndex(cols) j = 0; j < cols.size(); ++j)
            _dof_number_to_value[i].insert(
                std::make_pair(cols[j], values[j] / _var_objects[i]->scalingFactor()));
        }
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
      if (_ping_pong_secondary_node_to_primary_node.find(_current_node->id()) ==
          _ping_pong_secondary_node_to_primary_node.end())
      {
        if (_contact_lm > -_pinfo->_distance)
        {
          // Ok, our math is telling us we should apply the constraint, but what if we are
          // ping-ponging back and forth between different primary faces?

          // This only works for a basic line search! Write assertion here
          if (_subproblem.computingNonlinearResid())
          {
            auto & primary_elem_sequence = _node_to_primary_elem_sequence[_current_node->id()];
            mooseAssert(
                _current_primary == _pinfo->_elem,
                "The current primary element and the PenetrationInfo object's element should "
                "be the same");
            primary_elem_sequence.push_back(_pinfo->_elem);

            // 5 here is a heuristic choice. In testing, generally speaking, if a node ping-pongs
            // back and forth 5 times then it will ping pong indefinitely. However, if it goes 3
            // times for example, it is not guaranteed to ping-pong indefinitely and the Newton
            // iteration may naturally resolve the correct face dofs that need to be involved in the
            // constraint.
            if (_ping_pong_protection && primary_elem_sequence.size() >= 5 &&
                _pinfo->_elem == *(primary_elem_sequence.rbegin() + 2) &&
                _pinfo->_elem == *(primary_elem_sequence.rbegin() + 4) &&
                _pinfo->_elem != *(primary_elem_sequence.rbegin() + 1) &&
                *(primary_elem_sequence.rbegin() + 1) == *(primary_elem_sequence.rbegin() + 3))
            {
              // Ok we are ping-ponging. Determine the primary node
              auto primary_node =
                  _penetration_locator._nearest_node.nearestNode(_current_node->id());

              // Sanity checks
              mooseAssert(_pinfo->_elem->get_node_index(primary_node) != libMesh::invalid_uint,
                          "The primary node is not on the current element");
              mooseAssert((*(primary_elem_sequence.rbegin() + 1))->get_node_index(primary_node) !=
                              libMesh::invalid_uint,
                          "The primary node is not on the other ping-ponging element");

              _ping_pong_secondary_node_to_primary_node.insert(
                  std::make_pair(_current_node->id(), primary_node));
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
      auto it = _ping_pong_secondary_node_to_primary_node.find(_current_node->id());
      if (it != _ping_pong_secondary_node_to_primary_node.end())
      {
        _tie_nodes = true;
        _nearest_node = it->second;
        _primary_index = _current_primary->get_node_index(_nearest_node);
        mooseAssert(_primary_index != libMesh::invalid_uint,
                    "nearest node not a node on the current primary element");
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

  // If we're not applying the constraint then we can clear the node to primary elem sequence for
  // this node
  if (_node_to_primary_elem_sequence.find(_current_node->id()) !=
      _node_to_primary_elem_sequence.end())
    _node_to_primary_elem_sequence[_current_node->id()].clear();
  return false;
}

Real
RANFSNormalMechanicalContact::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Secondary:
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
          // The normal points out of the primary face
          return _contact_lm * -_pinfo->_normal(_component);
      }
    }

    case Moose::ConstraintType::Primary:
    {
      if (_tie_nodes)
      {
        if (_i == _primary_index)
          return _tied_lm;
        else
          return 0;
      }
      else
        return _test_primary[_i][_qp] * _contact_lm * _pinfo->_normal(_component);
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
    case Moose::ConstraintJacobianType::SecondarySecondary:
    {
      if (_tie_nodes)
        return _phi_secondary[_j][_qp];

      // doing contact
      else
      {
        // corresponds to gap equation
        if (_largest_component == static_cast<unsigned int>(_component))
          // _phi_secondary has been set such that it is 1 when _j corresponds to the degree of
          // freedom associated with the _current node and 0 otherwise
          return std::abs(_pinfo->_normal(_component)) * _phi_secondary[_j][_qp];

        // corresponds to regular residual with Lagrange Multiplier applied
        else
        {
          Real ret_val = 0;
          for (MooseIndex(_disp_coupling) i = 0; i < _disp_coupling.size(); ++i)
            if (_disp_coupling(_component, i))
            {
              mooseAssert(
                  _dof_number_to_value[i].find(_connected_dof_indices[_j]) !=
                      _dof_number_to_value[i].end(),
                  "The connected dof index is not found in the _dof_number_to_value container. "
                  "This must mean that insufficient sparsity was allocated");
              ret_val += -_pinfo->_normal(_component) * _pinfo->_normal(i) *
                         _dof_number_to_value[i][_connected_dof_indices[_j]];
            }
          return ret_val;
        }
      }
    }

    case Moose::ConstraintJacobianType::SecondaryPrimary:
    {
      if (_tie_nodes)
      {
        if (_primary_index == _j)
          return -1;

        // We're tying the secondary node to only one node on the primary side (specified by
        // _primary_index). If the current _j doesn't correspond to that tied primary node, then the
        // secondary residual doesn't depend on it
        else
          return 0;
      }
      else
      {
        if (_largest_component == static_cast<unsigned int>(_component))
          return -std::abs(_pinfo->_normal(_component)) * _phi_primary[_j][_qp];

        // If we're not applying the gap constraint equation on this _component, then we're
        // applying a Lagrange multiplier, and consequently there is no dependence of the secondary
        // residual on the primary dofs because the Lagrange multiplier is only a functon of the
        // secondary residuals
        else
          return 0;
      }
    }

    case Moose::ConstraintJacobianType::PrimarySecondary:
    {
      if (_tie_nodes)
      {
        if (_i == _primary_index)
        {
          mooseAssert(_dof_number_to_value[_component].find(_connected_dof_indices[_j]) !=
                          _dof_number_to_value[_component].end(),
                      "The connected dof index is not found in the _dof_number_to_value container. "
                      "This must mean that insufficient sparsity was allocated");
          return _dof_number_to_value[_component][_connected_dof_indices[_j]];
        }

        // We only apply the tied node Lagrange multiplier to the closest primary node
        else
          return 0;
      }
      else
      {
        Real ret_val = 0;
        for (MooseIndex(_disp_coupling) i = 0; i < _disp_coupling.size(); ++i)
          if (_disp_coupling(_component, i))
          {
            mooseAssert(
                _dof_number_to_value[i].find(_connected_dof_indices[_j]) !=
                    _dof_number_to_value[i].end(),
                "The connected dof index is not found in the _dof_number_to_value container. "
                "This must mean that insufficient sparsity was allocated");
            ret_val += _test_primary[_i][_qp] * _pinfo->_normal(_component) * _pinfo->_normal(i) *
                       _dof_number_to_value[i][_connected_dof_indices[_j]];
          }
        return ret_val;
      }
    }

      // The only primary-primary dependence would come from the dependence of the normal and also
      // the location of the integration (quadrature) points. We assume (valid or not) that this
      // dependence is weak
      // case MooseConstraintJacobianType::PrimaryPrimary

    default:
      return 0;
  }
}

void
RANFSNormalMechanicalContact::computeSecondaryValue(NumericVector<Number> &)
{
}

Real
RANFSNormalMechanicalContact::computeQpSecondaryValue()
{
  mooseError(
      "We overrode commputeSecondaryValue so computeQpSecondaryValue should never get called");
}
