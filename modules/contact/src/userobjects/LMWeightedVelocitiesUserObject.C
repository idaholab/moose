//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMWeightedVelocitiesUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "AutomaticMortarGeneration.h"
#include "ContactFrictionUtils.h"
#include "MortarContactUtils.h"

#include <algorithm>

registerMooseObject("ContactApp", LMWeightedVelocitiesUserObject);

InputParameters
LMWeightedVelocitiesUserObject::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params += LMWeightedGapUserObject::newParams();
  params.addClassDescription("Provides the mortar contact Lagrange multipliers (normal and "
                             "tangential) for constraint enforcement.");
  params.renameCoupledVar("lm_variable", "lm_variable_normal", "");
  params.addRequiredCoupledVar(
      "lm_variable_tangential_one",
      "The Lagrange multiplier variable representing the tangential contact pressure along the "
      "first tangential direction (the only one in two dimensions).");
  params.addCoupledVar("lm_variable_tangential_two",
                       "The Lagrange multiplier variable representing the tangential contact "
                       "pressure along the second tangential direction.");
  return params;
}

LMWeightedVelocitiesUserObject::LMWeightedVelocitiesUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    LMWeightedGapUserObject(parameters),
    _lm_variable_tangential_one(getVar("lm_variable_tangential_one", 0)),
    _lm_variable_tangential_two(isParamValid("lm_variable_tangential_two")
                                    ? getVar("lm_variable_tangential_two", 0)
                                    : nullptr)
{
  // Check that user inputted a variable
  checkInput(_lm_variable_tangential_one, "lm_variable_tangential_one");
  if (_lm_variable_tangential_two)
    checkInput(_lm_variable_tangential_two, "lm_variable_tangential_two");

  // Check that user inputted the right type of variable
  verifyLagrange(*_lm_variable_tangential_one, "lm_variable_tangential_one");
  if (_lm_variable_tangential_two)
    verifyLagrange(*_lm_variable_tangential_two, "lm_variable_tangential_two");
}

void
LMWeightedVelocitiesUserObject::requestFrictionRegularizationData(const bool elastic_slip)
{
  if (!_compute_friction_regularization_data)
  {
    _secondary_x_old = &_disp_x_var->slnOld();
    _primary_x_old = &_disp_x_var->slnOldNeighbor();
    _secondary_y_old = &_disp_y_var->slnOld();
    _primary_y_old = &_disp_y_var->slnOldNeighbor();
    if (_has_disp_z)
    {
      _secondary_z_old = &_disp_z_var->slnOld();
      _primary_z_old = &_disp_z_var->slnOldNeighbor();
    }
    _compute_friction_regularization_data = true;
  }
  _compute_elastic_slip_data = _compute_elastic_slip_data || elastic_slip;
}

ADRealVectorValue
LMWeightedVelocitiesUserObject::tangentialDisplacementIncrement(const DofObject * const dof) const
{
  const auto & increment = libmesh_map_find(_dof_to_tangential_displacement_increment, dof);
  const auto normalization = libmesh_map_find(_dof_to_weighted_gap, dof).second;
  const auto & frame = libmesh_map_find(_dof_to_contact_frame, dof).constraint;
  ADRealVectorValue tangential_increment = increment[0] / normalization * frame[0];
  if (_3d)
    tangential_increment += increment[1] / normalization * frame[1];
  return tangential_increment;
}

ADReal
LMWeightedVelocitiesUserObject::tangentialSlipIncrement(const DofObject * const dof) const
{
  const auto & increment = libmesh_map_find(_dof_to_tangential_displacement_increment, dof);
  const auto normalization = libmesh_map_find(_dof_to_weighted_gap, dof).second;
  return Moose::Contact::tangentialSlipMagnitude(
      {increment[0] / normalization, increment[1] / normalization, 0.0});
}

const LMWeightedVelocitiesUserObject::ContactFrameData &
LMWeightedVelocitiesUserObject::contactFrames(const DofObject * const dof) const
{
  return libmesh_map_find(_dof_to_contact_frame, dof);
}

void
LMWeightedVelocitiesUserObject::computeQpProperties()
{
  WeightedVelocitiesUserObject::computeQpProperties();
  if (!_compute_friction_regularization_data)
    return;

  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);
  std::array<const MooseVariable *, 3> variables{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_increment{
      {_primary_disp_x[_qp] - (*_primary_x_old)[_qp],
       _primary_disp_y[_qp] - (*_primary_y_old)[_qp],
       _has_disp_z ? (*_primary_disp_z)[_qp] - (*_primary_z_old)[_qp] : 0.0}};
  std::array<ADReal, 3> secondary_increment{
      {_secondary_disp_x[_qp] - (*_secondary_x_old)[_qp],
       _secondary_disp_y[_qp] - (*_secondary_y_old)[_qp],
       _has_disp_z ? (*_secondary_disp_z)[_qp] - (*_secondary_z_old)[_qp] : 0.0}};
  trimInteriorNodeDerivatives(primary_ip_lowerd_map, variables, primary_increment, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, variables, secondary_increment, true);
  _qp_relative_displacement_increment = {secondary_increment[0] - primary_increment[0],
                                         secondary_increment[1] - primary_increment[1],
                                         secondary_increment[2] - primary_increment[2]};
}

void
LMWeightedVelocitiesUserObject::computeQpIProperties()
{
  WeightedVelocitiesUserObject::computeQpIProperties();
  if (!_compute_friction_regularization_data)
    return;

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
  mooseAssert(_is_weighted_gap_nodal,
              "Friction regularization requires nodal mortar Lagrange multipliers");
  const DofObject * const dof = _lower_secondary_elem->node_ptr(_i);
  const ADReal weight = (*_test)[_i][_qp] * _JxW_msm[_qp] * _coord[_qp];
  auto & tangential_increment = _dof_to_tangential_displacement_increment[dof];
  tangential_increment[0] += weight * _qp_relative_displacement_increment * nodal_tangents[0][_i];
  if (_3d)
    tangential_increment[1] += weight * _qp_relative_displacement_increment * nodal_tangents[1][_i];
}

void
LMWeightedVelocitiesUserObject::buildContactFrames()
{
  _dof_to_contact_frame.clear();
  mooseAssert(_nodal, "Friction regularization currently requires nodal mortar LM variables");

  for (const auto & [dof, increment] : _dof_to_tangential_displacement_increment)
  {
    libmesh_ignore(increment);
    if (dof->processor_id() != processor_id())
      continue;

    const auto * const node = static_cast<const Node *>(dof);
    const auto & incident_elems = libmesh_map_find(amg().nodesToSecondaryElem(), node->id());
    const Elem * const source_elem = *std::min_element(incident_elems.begin(),
                                                       incident_elems.end(),
                                                       [](const Elem * left, const Elem * right)
                                                       { return left->id() < right->id(); });
    const auto local_node = source_elem->get_node_index(node);
    mooseAssert(local_node != libMesh::invalid_uint,
                "The elastic-slip frame source must contain the mortar node");
    const Point reference_point = source_elem->master_point(local_node);
    const auto nodal_normals = amg().getNodalNormals(*source_elem);
    const auto nodal_tangents = amg().getNodalTangents(*source_elem);
    const RealVectorValue preferred_normal = nodal_normals[local_node];
    Moose::Contact::ContactTangentialFrame constraint_frame{
        {nodal_tangents[0][local_node], nodal_tangents[1][local_node], preferred_normal}};

    auto material_frame = Moose::Contact::buildContactTangentialFrame(
        *source_elem, reference_point, preferred_normal);
    _dof_to_contact_frame[dof] = {std::move(material_frame), std::move(constraint_frame)};
  }
}

void
LMWeightedVelocitiesUserObject::initialize()
{
  WeightedVelocitiesUserObject::initialize();
  _dof_to_tangential_displacement_increment.clear();
  _dof_to_contact_frame.clear();
}

void
LMWeightedVelocitiesUserObject::finalize()
{
  WeightedVelocitiesUserObject::finalize();

  if (_compute_friction_regularization_data)
    Moose::Mortar::Contact::communicateVelocities(_dof_to_tangential_displacement_increment,
                                                  _subproblem.mesh(),
                                                  _nodal,
                                                  _communicator,
                                                  false);
  if (_compute_elastic_slip_data)
    buildContactFrames();
}

const ADVariableValue &
LMWeightedVelocitiesUserObject::contactTangentialPressureDirOne() const
{
  return _lm_variable_tangential_one->adSlnLower();
}

const ADVariableValue &
LMWeightedVelocitiesUserObject::contactTangentialPressureDirTwo() const
{
  return _lm_variable_tangential_two->adSlnLower();
}
