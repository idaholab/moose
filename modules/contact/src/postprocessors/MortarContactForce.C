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

#include "libmesh/parallel_algebra.h"

registerMooseObject("ContactApp", MortarContactForce);

InputParameters
MortarContactForce::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Normal contact force that one mortar contact pair transmits to its "
                             "secondary surface.");
  params.addRequiredParam<UserObjectName>(
      "weighted_gap_uo",
      "The weighted gap user object of the contact pair. The Contact action names it "
      "lm_weightedgap_object_<block name> (mortar) or penalty_weightedgap_object_<block name> "
      "(mortar_penalty), with the suffix _p_<primary>_s_<secondary> when the block has several "
      "pairs.");
  MooseEnum component("x y z magnitude", "magnitude");
  params.addParam<MooseEnum>(
      "component", component, "The force component to report, or its magnitude.");
  return params;
}

MortarContactForce::MortarContactForce(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _weighted_gap_uo(getUserObject<WeightedGapUserObject>("weighted_gap_uo")),
    _component(getParam<MooseEnum>("component")),
    _displaced_mesh(_fe_problem.getDisplacedProblem()->mesh()),
    _amg(_fe_problem.getMortarInterface(
        {_displaced_mesh.getBoundaryID(
             _weighted_gap_uo.parameters().get<BoundaryName>("primary_boundary")),
         _displaced_mesh.getBoundaryID(
             _weighted_gap_uo.parameters().get<BoundaryName>("secondary_boundary"))},
        {_weighted_gap_uo.primarySubdomain(), _weighted_gap_uo.secondarySubdomain()},
        /*on_displaced=*/true)),
    _node_to_elems(_amg.nodesToSecondaryElem())
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
  // F = sum_j p_j n_j int N_j, with the normalization int N_j over the mortar segments (a dual
  // basis function has the same integral as N_j)
  for (const auto & [dof_object, gap_and_normalization] : _weighted_gap_uo.dofToWeightedGap())
  {
    if (dof_object->processor_id() != processor_id())
      continue;
    const Node * const node = _displaced_mesh.nodePtr(dof_object->id());
    const Elem & elem = *libmesh_map_find(_node_to_elems, node->id()).front();
    const auto normal = _amg.getNodalNormals(elem)[elem.get_node_index(node)];
    const auto weighted_pressure =
        _weighted_gap_uo.getNormalContactPressure(node) * gap_and_normalization.second;
    if (_component < 3)
      _force(_component) += weighted_pressure * normal(_component);
    else
      _force += weighted_pressure * normal;
  }
}

void
MortarContactForce::finalize()
{
  if (_component < 3)
    gatherSum(_force(_component));
  else
    gatherSum(_force);
}

PostprocessorValue
MortarContactForce::getValue() const
{
  return _component == 3 ? _force.norm() : _force(_component);
}
