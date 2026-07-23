//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "NodalNormalDerivativesTest.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "MortarInterfaceWarehouse.h"
#include "WeightedGapUserObject.h"
#include "WeightedVelocitiesUserObject.h"

#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <set>

registerMooseObject("ContactTestApp", NodalNormalDerivativesTest);

namespace
{
bool
closeEnough(const Real actual, const Real expected, const Real tolerance = 1e-11)
{
  return std::abs(actual - expected) <= tolerance * (1.0 + std::abs(expected));
}

bool
hasDerivativeIndex(const ADReal & value, const dof_id_type dof)
{
  const auto & indices = value.derivatives().nude_indices();
  return std::find(indices.begin(), indices.end(), dof) != indices.end();
}

bool
hasNonzeroDerivativeIndex(const ADReal & value, const dof_id_type dof)
{
  return hasDerivativeIndex(value, dof) && std::abs(value.derivatives()[dof]) > TOLERANCE;
}

bool
hasDerivatives(const ADRealVectorValue & value)
{
  for (const auto component : make_range(Moose::dim))
    if (value(component).derivatives().size())
      return true;
  return false;
}

bool
hasDerivatives(const ADReal & value)
{
  return value.derivatives().size();
}

bool
hasDerivatives(const std::array<ADRealVectorValue, 2> & values)
{
  return hasDerivatives(values[0]) || hasDerivatives(values[1]);
}
} // namespace

InputParameters
NodalNormalDerivativesTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>("weighted_gap_uo",
                                          "Mechanical mortar weighted-gap user object to inspect.");
  params.addRequiredParam<VariableName>("disp_x", "The x displacement variable used by contact.");
  params.addRequiredParam<VariableName>("disp_y", "The y displacement variable used by contact.");
  params.addParam<VariableName>("disp_z", "The z displacement variable used by contact.");
  params.addParam<unsigned int>(
      "max_derivative_dependencies",
      "Optional upper bound on the number of derivative entries stored by any scalar component "
      "of the differentiated nodal normal or tangent basis.");
  params.addParam<bool>(
      "require_distributed_velocity_derivatives",
      false,
      "Require a partition-split secondary node star whose owning process receives nonzero "
      "weighted-velocity derivatives from nodes exclusive to a remote incident face.");
  params.addParam<bool>(
      "error_after_jacobian_assembly",
      false,
      "Raise an expected test error at a nonlinear convergence check after the diagnostic has "
      "observed Jacobian-mode contact geometry. This proves the contact constraints assembled.");
  params.set<ExecFlagEnum>("execute_on") = {
      EXEC_LINEAR, EXEC_NONLINEAR, EXEC_NONLINEAR_CONVERGENCE};
  params.addClassDescription(
      "Checks stored values, displacement derivative stencils, and tangent linearization for "
      "mechanical-contact nodal geometry.");
  return params;
}

NodalNormalDerivativesTest::NodalNormalDerivativesTest(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _weighted_gap_uo(getUserObject<WeightedGapUserObject>("weighted_gap_uo")),
    _weighted_velocities_uo(dynamic_cast<const WeightedVelocitiesUserObject *>(&_weighted_gap_uo)),
    _disp_x_var(dynamic_cast<MooseVariable *>(
        &_subproblem.getVariable(0, getParam<VariableName>("disp_x")))),
    _disp_y_var(dynamic_cast<MooseVariable *>(
        &_subproblem.getVariable(0, getParam<VariableName>("disp_y")))),
    _disp_z_var(isParamValid("disp_z") ? dynamic_cast<MooseVariable *>(&_subproblem.getVariable(
                                             0, getParam<VariableName>("disp_z")))
                                       : nullptr),
    _require_distributed_velocity_derivatives(
        getParam<bool>("require_distributed_velocity_derivatives")),
    _error_after_jacobian_assembly(getParam<bool>("error_after_jacobian_assembly"))
{
  if (!_disp_x_var)
    paramError("disp_x", "NodalNormalDerivativesTest requires a finite-element variable.");
  if (!_disp_y_var)
    paramError("disp_y", "NodalNormalDerivativesTest requires a finite-element variable.");
  if (isParamValid("disp_z") && !_disp_z_var)
    paramError("disp_z", "NodalNormalDerivativesTest requires a finite-element variable.");
  if (_require_distributed_velocity_derivatives && !_weighted_velocities_uo)
    paramError("require_distributed_velocity_derivatives",
               "Distributed weighted-velocity checks require a WeightedVelocitiesUserObject.");
  if (_require_distributed_velocity_derivatives && !_disp_z_var)
    paramError("require_distributed_velocity_derivatives",
               "Distributed weighted-velocity checks require three displacement variables.");
}

void
NodalNormalDerivativesTest::execute()
{
  // A nonlinear convergence check follows residual/Jacobian assembly. Delay the expected test
  // termination until this stage so strict-preallocation tests exercise the production constraints,
  // not only the user objects that provide their contact geometry.
  if (_current_execute_flag == EXEC_NONLINEAR_CONVERGENCE)
  {
    if (_error_after_jacobian_assembly && _verified_jacobian_geometry)
      mooseError("Nodal-normal derivative checks completed after contact Jacobian assembly.");
    return;
  }

  // This diagnostic only needs one real residual -> Jacobian -> residual sequence. Later Newton
  // iterates may legitimately have no mortar overlap, so retire the geometry checks once the
  // value, derivative, and cache-mode contracts have all been verified.
  if (_verified_mode_transition && _verified_jacobian_geometry)
    return;

  const auto & interfaces = _fe_problem.getMortarInterfaces(/*displaced=*/true);
  if (interfaces.size() != 1)
    mooseError(
        "NodalNormalDerivativesTest expects exactly one displaced mortar interface, but found ",
        interfaces.size(),
        ".");

  auto & amg = *interfaces.begin()->second.amg;
  // Inspect every unique secondary node represented by the active mortar segment mesh. In
  // particular, this includes corner, midside, and face-center nodes on quadratic interfaces.
  std::map<dof_id_type, const Elem *> secondary_node_to_elem;
  for (const auto & [segment, segment_info] : amg.mortarSegmentMeshElemToInfo())
    if (!_verified_mode_transition)
    {
      libmesh_ignore(segment);
      if (segment_info.secondary_elem)
        for (const auto i : make_range(segment_info.secondary_elem->n_nodes()))
          secondary_node_to_elem.emplace(segment_info.secondary_elem->node_id(i),
                                         segment_info.secondary_elem);
    }

  unsigned int found_secondary_nodes = !secondary_node_to_elem.empty();
  _communicator.max(found_secondary_nodes);
  if (!found_secondary_nodes)
    mooseError("Unable to find lower-dimensional secondary nodes to inspect.");

  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);

  {
    const auto check_mode = [&](const bool expect_derivatives)
    {
      bool found_normal_derivatives = false;
      bool found_tangent_derivatives = false;
      for (const auto & [secondary_node_id, secondary_elem] : secondary_node_to_elem)
      {
        unsigned int i = libMesh::invalid_uint;
        for (const auto candidate : secondary_elem->node_index_range())
          if (secondary_elem->node_id(candidate) == secondary_node_id)
          {
            i = candidate;
            break;
          }
        if (i == libMesh::invalid_uint)
          mooseError("Unable to locate a cache-mode probe node on its secondary element.");

        const auto raw_normals = amg.getNodalNormals(*secondary_elem);
        const auto raw_tangents = amg.getNodalTangents(*secondary_elem);
        const auto & normal = _mode_cache.normal(
            amg, *secondary_elem, i, _disp_x_var, _disp_y_var, _disp_z_var, expect_derivatives);
        const auto & tangents = _mode_cache.tangents(
            amg, *secondary_elem, i, _disp_x_var, _disp_y_var, _disp_z_var, expect_derivatives);
        if ((MetaPhysicL::raw_value(normal) - raw_normals[i]).norm() > TOLERANCE)
          mooseError("Changing the AD derivative mode changed a cached nodal-normal value.");
        for (const auto direction : make_range(tangents.size()))
          if ((MetaPhysicL::raw_value(tangents[direction]) - raw_tangents[direction][i]).norm() >
              TOLERANCE)
            mooseError("Changing the AD derivative mode changed a cached nodal-tangent value.");

        found_normal_derivatives |= hasDerivatives(normal);
        found_tangent_derivatives |= hasDerivatives(tangents);
        if (!expect_derivatives && (found_normal_derivatives || found_tangent_derivatives))
          mooseError(
              "The nodal-geometry cache retained derivatives while AD derivatives were off.");
      }

      unsigned int found_normal_derivatives_anywhere = found_normal_derivatives;
      unsigned int found_tangent_derivatives_anywhere = found_tangent_derivatives;
      _communicator.max(found_normal_derivatives_anywhere);
      _communicator.max(found_tangent_derivatives_anywhere);
      if (expect_derivatives &&
          (!found_normal_derivatives_anywhere || !found_tangent_derivatives_anywhere))
        mooseError("The nodal-geometry cache did not follow the active AD derivative mode.");
    };

    check_mode(do_derivatives);
    if (do_derivatives)
      _saw_jacobian_mode = _saw_residual_mode;
    else
    {
      if (_saw_jacobian_mode)
      {
        _console << "Verified false-true-false AD nodal-geometry cache modes" << std::endl;
        _verified_mode_transition = true;
        _mode_cache.clear();
      }
      _saw_residual_mode = true;
    }
  }

  // Residual evaluations intentionally carry values only. Verify that the production maps also
  // respect that contract, then leave the derivative checks for a Jacobian evaluation.
  if (!do_derivatives)
  {
    const auto & differentiated_gaps = _weighted_gap_uo.dofToWeightedGap();
    for (const auto & [dof, differentiated_gap] : differentiated_gaps)
    {
      libmesh_ignore(dof);
      if (hasDerivatives(differentiated_gap.first))
        mooseError("A residual-only weighted gap retained AD derivatives.");
    }

    if (_weighted_velocities_uo)
      for (const auto & [dof, velocities] : _weighted_velocities_uo->dofToWeightedVelocities())
      {
        libmesh_ignore(dof);
        for (const auto & velocity : velocities)
          if (hasDerivatives(velocity))
            mooseError("A residual-only weighted velocity retained AD derivatives.");
      }

    _console << "Verified residual-only contact geometry has no AD derivatives" << std::endl;
    return;
  }

  if (_verified_jacobian_geometry)
    return;

  Moose::Mortar::Contact::NodalNormalDerivativeCache geometry_cache;
  const std::array<const MooseVariable *, 3> displacement_variables = {
      {_disp_x_var, _disp_y_var, _disp_z_var}};

  bool found_nonzero_derivative = false;
  bool found_nonzero_tangent_derivative = false;
  std::array<bool, 2> found_nonzero_tangent_derivative_by_direction = {{false, false}};
  unsigned int max_derivative_dependencies = 0;
  std::optional<ADRealVectorValue> probe_normal;
  std::optional<std::array<ADRealVectorValue, 2>> probe_tangents;
  dof_id_type probe_dof = DofObject::invalid_id;
  dof_id_type probe_secondary_node_id = DofObject::invalid_id;
  Real probe_score = -1.0;
  for (const auto & [secondary_node_id, secondary_elem] : secondary_node_to_elem)
  {
    unsigned int i = libMesh::invalid_uint;
    for (const auto candidate : make_range(secondary_elem->n_nodes()))
      if (secondary_elem->node_id(candidate) == secondary_node_id)
      {
        i = candidate;
        break;
      }
    if (i == libMesh::invalid_uint)
      mooseError("Secondary element ",
                 secondary_elem->id(),
                 " does not contain mapped node ",
                 secondary_node_id,
                 ".");

    const auto raw_normals = amg.getNodalNormals(*secondary_elem);
    const auto raw_tangents = amg.getNodalTangents(*secondary_elem);
    const auto & ad_normal = geometry_cache.normal(
        amg, *secondary_elem, i, _disp_x_var, _disp_y_var, _disp_z_var, do_derivatives);
    if ((MetaPhysicL::raw_value(ad_normal) - raw_normals[i]).norm() > TOLERANCE)
      mooseError("Contact normal with derivatives ",
                 i,
                 " does not preserve the stored mortar nodal-normal value.");

    std::array<std::map<dof_id_type, Real>, 3> expected_normal_derivatives;
    for (const auto & sensitivity :
         amg.getNodalNormalCoordinateSensitivity(*secondary_elem->node_ptr(i)))
    {
      for (const auto coordinate : make_range(Moose::dim))
      {
        const auto * const variable = displacement_variables[coordinate];
        if (!variable || !sensitivity.node->n_dofs(variable->sys().number(), variable->number()))
          continue;
        const auto dof =
            sensitivity.node->dof_number(variable->sys().number(), variable->number(), 0);
        for (const auto component : make_range(Moose::dim))
          expected_normal_derivatives[component][dof] +=
              sensitivity.dnormal_dnode_coordinate(component, coordinate);
      }
    }

    for (const auto component : make_range(Moose::dim))
    {
      max_derivative_dependencies =
          std::max(max_derivative_dependencies,
                   cast_int<unsigned int>(ad_normal(component).derivatives().size()));
      for (const auto & [dof, expected] : expected_normal_derivatives[component])
      {
        const Real actual = ad_normal(component).derivatives()[dof];
        if (!closeEnough(actual, expected))
          mooseError("Normal derivative mismatch for component ",
                     component,
                     " and displacement dof ",
                     dof,
                     ": expected ",
                     expected,
                     " but found ",
                     actual,
                     ".");
        if (expected == 0.0 && hasDerivativeIndex(ad_normal(component), dof))
          mooseError(
              "An exactly zero normal sensitivity was stored for displacement dof ", dof, ".");
        if (std::abs(expected) > TOLERANCE)
          found_nonzero_derivative = true;
      }

      for (const auto dof : ad_normal(component).derivatives().nude_indices())
      {
        const auto expected_it = expected_normal_derivatives[component].find(dof);
        if (expected_it == expected_normal_derivatives[component].end() ||
            expected_it->second == 0.0)
          mooseError("Unexpected normal derivative index ", dof, " for component ", component, ".");
      }
    }

    std::set<dof_id_type> normal_derivative_dofs;
    for (const auto component : make_range(Moose::dim))
      for (const auto dof : ad_normal(component).derivatives().nude_indices())
        normal_derivative_dofs.insert(dof);
    for (const auto dof : normal_derivative_dofs)
    {
      Point dnormal;
      for (const auto component : make_range(Moose::dim))
        dnormal(component) = ad_normal(component).derivatives()[dof];
      const Real chart_distance = std::abs(raw_normals[i](0) + 1.0);
      const Real score = chart_distance / (1.0 + dnormal.norm());
      if (dnormal.norm() > TOLERANCE && chart_distance >= TOLERANCE && score > probe_score)
      {
        probe_score = score;
        probe_normal = ad_normal;
        probe_dof = dof;
        probe_secondary_node_id = secondary_node_id;
      }
    }

    const auto & ad_tangents = geometry_cache.tangents(
        amg, *secondary_elem, i, _disp_x_var, _disp_y_var, _disp_z_var, do_derivatives);
    for (const auto direction : make_range(ad_tangents.size()))
    {
      if ((MetaPhysicL::raw_value(ad_tangents[direction]) - raw_tangents[direction][i]).norm() >
          TOLERANCE)
        mooseError("Contact tangent with derivatives ",
                   direction,
                   " for node ",
                   i,
                   " does not preserve the stored mortar nodal-tangent value.");

      for (const auto component : make_range(Moose::dim))
      {
        max_derivative_dependencies = std::max(
            max_derivative_dependencies,
            cast_int<unsigned int>(ad_tangents[direction](component).derivatives().size()));
        for (const auto derivative : ad_tangents[direction](component).derivatives().nude_data())
        {
          found_nonzero_tangent_derivative =
              found_nonzero_tangent_derivative || std::abs(derivative) > TOLERANCE;
          found_nonzero_tangent_derivative_by_direction[direction] =
              found_nonzero_tangent_derivative_by_direction[direction] ||
              std::abs(derivative) > TOLERANCE;
        }
      }
    }

    if (probe_secondary_node_id == secondary_node_id)
      probe_tangents = ad_tangents;
  }

  unsigned int found_nonzero_derivative_anywhere = found_nonzero_derivative;
  unsigned int found_nonzero_tangent_derivative_anywhere = found_nonzero_tangent_derivative;
  _communicator.max(found_nonzero_derivative_anywhere);
  _communicator.max(found_nonzero_tangent_derivative_anywhere);
  if (!found_nonzero_derivative_anywhere)
    mooseError(
        "No displacement derivative was found in the contact nodal normals with derivatives.");

  if (!found_nonzero_tangent_derivative_anywhere)
    mooseError("No displacement derivative was found in the contact nodal tangents with "
               "derivatives.");

  if (_weighted_velocities_uo && _disp_z_var)
    for (const auto direction : make_range(found_nonzero_tangent_derivative_by_direction.size()))
    {
      unsigned int found_direction_anywhere =
          found_nonzero_tangent_derivative_by_direction[direction];
      _communicator.max(found_direction_anywhere);
      if (!found_direction_anywhere)
        mooseError("Three-dimensional friction did not differentiate nodal tangent direction ",
                   direction,
                   ".");
    }

  unsigned int found_probe = probe_normal && probe_tangents && probe_dof != DofObject::invalid_id;
  _communicator.max(found_probe);
  if (!found_probe)
    mooseError("Unable to select a nonsingular nodal-normal derivative for tangent checks.");

  _communicator.max(max_derivative_dependencies);
  if (isParamValid("max_derivative_dependencies") &&
      max_derivative_dependencies > getParam<unsigned int>("max_derivative_dependencies"))
    paramError("max_derivative_dependencies",
               "The measured maximum dependency count was ",
               max_derivative_dependencies,
               ".");

  _console << "Maximum nodal-geometry derivative dependency count: " << max_derivative_dependencies
           << std::endl;

  if (_require_distributed_velocity_derivatives)
  {
    unsigned int found_split_star = 0;
    const auto rank = _communicator.rank();
    for (const auto & [node_id, secondary_elem] : secondary_node_to_elem)
    {
      const Node * secondary_node = nullptr;
      for (const auto i : secondary_elem->node_index_range())
        if (secondary_elem->node_id(i) == node_id)
        {
          secondary_node = secondary_elem->node_ptr(i);
          break;
        }
      if (!secondary_node || secondary_node->processor_id() != rank)
        continue;

      for (const auto & sensitivity : amg.getNodalNormalCoordinateSensitivity(*secondary_node))
        if (sensitivity.node->processor_id() != rank)
        {
          found_split_star = 1;
          break;
        }
    }
    _communicator.max(found_split_star);
    if (!found_split_star)
      mooseError("The distributed contact test did not split a secondary node star.");
  }

  if (probe_normal && probe_tangents && probe_dof != DofObject::invalid_id)
  {
    const Point normal = MetaPhysicL::raw_value(*probe_normal);
    Point dnormal;
    for (const auto component : make_range(Moose::dim))
      dnormal(component) = (*probe_normal)(component).derivatives()[probe_dof];

    const Real fd_step = 1e-7;
    const Point normal_plus = normal + fd_step * dnormal;
    const Point normal_minus = normal - fd_step * dnormal;
    const auto tangent_plus = Moose::Mortar::householderTangents(normal_plus);
    const auto tangent_minus = Moose::Mortar::householderTangents(normal_minus);
    std::array<Point, 2> dtangent;
    for (const auto direction : make_range(dtangent.size()))
      for (const auto component : make_range(Moose::dim))
      {
        dtangent[direction](component) =
            (tangent_plus[direction](component) - tangent_minus[direction](component)) /
            (2.0 * fd_step);
        const Real actual = (*probe_tangents)[direction](component).derivatives()[probe_dof];
        if (!closeEnough(actual, dtangent[direction](component), 1e-7))
          mooseError("Householder tangent derivative mismatch for direction ",
                     direction,
                     ", component ",
                     component,
                     ", and displacement dof ",
                     probe_dof,
                     ": expected ",
                     dtangent[direction](component),
                     " but found ",
                     actual,
                     ".");
      }
  }

  if (_weighted_velocities_uo)
  {
    const auto & differentiated_velocities = _weighted_velocities_uo->dofToWeightedVelocities();
    const auto expected_directions = _disp_z_var ? 2 : 1;
    std::array<unsigned int, 2> found_velocity_derivative = {{0, 0}};
    for (const auto & [dof, differentiated_velocity] : differentiated_velocities)
    {
      libmesh_ignore(dof);

      for (const auto direction : make_range(expected_directions))
        for (const auto derivative : differentiated_velocity[direction].derivatives().nude_data())
          found_velocity_derivative[direction] =
              found_velocity_derivative[direction] || std::abs(derivative) > TOLERANCE;
    }

    for (const auto direction : make_range(expected_directions))
    {
      _communicator.max(found_velocity_derivative[direction]);
      if (!found_velocity_derivative[direction])
        mooseError("No derivative was found in production weighted-velocity tangent direction ",
                   direction,
                   ".");
    }
    _console << "Verified production weighted-velocity values and derivatives in "
             << expected_directions << " tangent directions" << std::endl;

    if (_require_distributed_velocity_derivatives)
    {
      std::array<unsigned int, 2> found_remote_derivative = {{0, 0}};
      std::array<Real, 2> max_weighted_velocity = {{0, 0}};
      const std::array<const MooseVariable *, 3> displacement_variables = {
          {_disp_x_var, _disp_y_var, _disp_z_var}};
      const auto rank = _communicator.rank();

      for (const auto & [dof, velocities] : differentiated_velocities)
      {
        libmesh_ignore(dof);
        for (const auto direction : make_range(expected_directions))
          max_weighted_velocity[direction] =
              std::max(max_weighted_velocity[direction],
                       std::abs(MetaPhysicL::raw_value(velocities[direction])));
      }

      for (const auto & [node_id, secondary_elem] : secondary_node_to_elem)
      {
        unsigned int secondary_node_index = libMesh::invalid_uint;
        for (const auto candidate : secondary_elem->node_index_range())
          if (secondary_elem->node_id(candidate) == node_id)
          {
            secondary_node_index = candidate;
            break;
          }
        if (secondary_node_index == libMesh::invalid_uint)
          mooseError("Unable to locate a split-star node on its secondary element.");
        const Node * const secondary_node = secondary_elem->node_ptr(secondary_node_index);
        if (!secondary_node || secondary_node->processor_id() != rank)
          continue;

        const auto velocity_it = differentiated_velocities.find(secondary_node);
        if (velocity_it == differentiated_velocities.end())
          continue;

        for (const auto & sensitivity : amg.getNodalNormalCoordinateSensitivity(*secondary_node))
        {
          if (sensitivity.node->processor_id() != rank)
          {
            for (const auto * const variable : displacement_variables)
            {
              if (!variable ||
                  !sensitivity.node->n_dofs(variable->sys().number(), variable->number()))
                continue;
              const auto dof =
                  sensitivity.node->dof_number(variable->sys().number(), variable->number(), 0);
              for (const auto direction : make_range(expected_directions))
                found_remote_derivative[direction] =
                    found_remote_derivative[direction] ||
                    hasNonzeroDerivativeIndex(velocity_it->second[direction], dof);
            }
          }
        }
      }

      // The first Jacobian can precede any tangential update, in which case tangent-basis
      // derivatives correctly contract with a zero velocity. Defer the communication check until
      // both sliding directions are active rather than interpreting that zero as missing data.
      bool active_sliding = true;
      for (const auto direction : make_range(expected_directions))
      {
        _communicator.max(max_weighted_velocity[direction]);
        active_sliding = active_sliding && max_weighted_velocity[direction] > TOLERANCE;
      }
      if (!active_sliding)
        return;

      for (const auto direction : make_range(expected_directions))
      {
        _communicator.max(found_remote_derivative[direction]);
        if (!found_remote_derivative[direction])
          mooseError("No communicated weighted-velocity derivative was found in tangent "
                     "direction ",
                     direction,
                     ".");
      }
      _console << "Verified split-star weighted-velocity derivative communication" << std::endl;
    }
  }

  _console << "Verified mechanical-contact nodal normals preserve stored values with "
              "displacement derivatives"
           << std::endl;

  _console << "Verified mechanical-contact nodal tangents preserve stored values with "
              "displacement derivatives"
           << std::endl;
  _verified_jacobian_geometry = true;
}
