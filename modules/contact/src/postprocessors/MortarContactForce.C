//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactForce.h"
#include "WeightedGapUserObject.h"
#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"

registerMooseObject("ContactApp", MortarContactForce);

InputParameters
MortarContactForce::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Total normal contact force that one mortar contact pair transmits to its secondary "
      "surface, summed from the nodal contact pressures, nodal normals and mortar weights of the "
      "pair's weighted gap user object.");
  params.addRequiredParam<UserObjectName>(
      "weighted_gap_uo",
      "The weighted gap user object of the contact pair. The Contact action names it "
      "lm_weightedgap_object_<block name> for a single pair and adds the pair suffix "
      "_p_<primary>_s_<secondary> when the block has several pairs.");
  MooseEnum component("x y z magnitude", "magnitude");
  params.addParam<MooseEnum>(
      "component", component, "The force component to report, or its magnitude.");
  return params;
}

MortarContactForce::MortarContactForce(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _weighted_gap_uo(getUserObject<WeightedGapUserObject>("weighted_gap_uo")),
    _component(getParam<MooseEnum>("component"))
{
}

void
MortarContactForce::initialize()
{
  _force.zero();
}

void
MortarContactForce::execute()
{
  const auto & uo_params = _weighted_gap_uo.parameters();
  const bool displaced = uo_params.get<bool>("use_displaced_mesh");
  const MooseMesh & mesh =
      displaced ? _fe_problem.getDisplacedProblem()->mesh() : _fe_problem.mesh();
  const auto & amg = _fe_problem.getMortarInterface(
      {mesh.getBoundaryID(uo_params.get<BoundaryName>("primary_boundary")),
       mesh.getBoundaryID(uo_params.get<BoundaryName>("secondary_boundary"))},
      {_weighted_gap_uo.primarySubdomain(), _weighted_gap_uo.secondarySubdomain()},
      displaced);

  // Nodal normals of the secondary surface, by node id
  std::unordered_map<dof_id_type, Point> normals;
  const auto & lm_mesh = mesh.getMesh();
  for (const auto * const elem :
       as_range(lm_mesh.active_subdomain_elements_begin(_weighted_gap_uo.secondarySubdomain()),
                lm_mesh.active_subdomain_elements_end(_weighted_gap_uo.secondarySubdomain())))
  {
    const auto elem_normals = amg.getNodalNormals(*elem);
    for (const auto n : make_range(elem->n_nodes()))
      normals.emplace(elem->node_id(n), elem_normals[n]);
  }

  // The traction on the secondary surface is sum_j Phi_j p_j n_j, so its integral is
  // sum_j p_j n_j int Phi_j. The weighted gap user object holds int Phi_j over the mortar segments
  // as the normalization of each secondary node, complete on the node's owner.
  const auto rank = processor_id();
  for (const auto & [dof_object, gap_and_normalization] : _weighted_gap_uo.dofToWeightedGap())
  {
    if (dof_object->processor_id() != rank)
      continue;
    const auto normal_it = normals.find(dof_object->id());
    if (normal_it == normals.end())
      mooseError("MortarContactForce requires a contact pressure at the secondary surface nodes.");
    const Node * const node = lm_mesh.node_ptr(dof_object->id());
    _force += _weighted_gap_uo.getNormalContactPressure(node) * gap_and_normalization.second *
              normal_it->second;
  }
}

void
MortarContactForce::finalize()
{
  for (const auto i : make_range(Moose::dim))
    gatherSum(_force(i));
}

PostprocessorValue
MortarContactForce::getValue() const
{
  return _component == 3 ? _force.norm() : _force(_component);
}
