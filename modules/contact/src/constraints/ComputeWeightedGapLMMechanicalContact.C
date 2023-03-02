//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeWeightedGapLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "MortarContactUtils.h"
#include "WeightedGapUserObject.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

registerMooseObject("ContactApp", ComputeWeightedGapLMMechanicalContact);

namespace
{
const InputParameters &
assignVarsInParams(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  const auto & disp_x_name = ret.get<std::vector<VariableName>>("disp_x");
  if (disp_x_name.size() != 1)
    mooseError("We require that the disp_x parameter have exactly one coupled name");

  // We do this so we don't get any variable errors during MortarConstraint(Base) construction
  ret.set<VariableName>("secondary_variable") = disp_x_name[0];
  ret.set<VariableName>("primary_variable") = disp_x_name[0];

  return ret;
}
}

InputParameters
ComputeWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription("Computes the weighted gap that will later be used to enforce the "
                             "zero-penetration mechanical contact conditions");
  params.suppressParameter<VariableName>("secondary_variable");
  params.suppressParameter<VariableName>("primary_variable");
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.set<bool>("use_displaced_mesh") = true;
  params.addRequiredParam<UserObjectName>("weighted_gap_uo", "The weighted gap user object");
  return params;
}

ComputeWeightedGapLMMechanicalContact::ComputeWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(assignVarsInParams(parameters)),
    _secondary_disp_x(adCoupledValue("disp_x")),
    _primary_disp_x(adCoupledNeighborValue("disp_x")),
    _secondary_disp_y(adCoupledValue("disp_y")),
    _primary_disp_y(adCoupledNeighborValue("disp_y")),
    _has_disp_z(isCoupled("disp_z")),
    _secondary_disp_z(_has_disp_z ? &adCoupledValue("disp_z") : nullptr),
    _primary_disp_z(_has_disp_z ? &adCoupledNeighborValue("disp_z") : nullptr),
    _c(getParam<Real>("c")),
    _normalize_c(getParam<bool>("normalize_c")),
    _nodal(getVar("disp_x", 0)->feType().family == LAGRANGE),
    _disp_x_var(getVar("disp_x", 0)),
    _disp_y_var(getVar("disp_y", 0)),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
    _weighted_gap_uo(getUserObject<WeightedGapUserObject>("weighted_gap_uo"))
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError(
        "use_displaced_mesh",
        "'use_displaced_mesh' must be true for the ComputeWeightedGapLMMechanicalContact object");

  if (!_var->isNodal())
    if (_var->feType().order != static_cast<Order>(0))
      mooseError("Normal contact constraints only support elemental variables of CONSTANT order");
}

ADReal ComputeWeightedGapLMMechanicalContact::computeQpResidual(Moose::MortarType)
{
  mooseError("We should never call computeQpResidual for ComputeWeightedGapLMMechanicalContact");
}

void
ComputeWeightedGapLMMechanicalContact::computeQpProperties()
{
  // Trim interior node variable derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp{
      {_primary_disp_x[_qp], _primary_disp_y[_qp], _has_disp_z ? (*_primary_disp_z)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp{{_secondary_disp_x[_qp],
                                        _secondary_disp_y[_qp],
                                        _has_disp_z ? (*_secondary_disp_z)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp, true);

  const ADReal & prim_x = primary_disp[0];
  const ADReal & prim_y = primary_disp[1];
  const ADReal * prim_z = nullptr;
  if (_has_disp_z)
    prim_z = &primary_disp[2];

  const ADReal & sec_x = secondary_disp[0];
  const ADReal & sec_y = secondary_disp[1];
  const ADReal * sec_z = nullptr;
  if (_has_disp_z)
    sec_z = &secondary_disp[2];

  // Compute gap vector
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();
  if (_has_disp_z)
    gap_vec(2).derivatives() = prim_z->derivatives() - sec_z->derivatives();

  // Compute integration point quantities
  if (_interpolate_normals)
    _qp_gap = gap_vec * (_normals[_qp] * _JxW_msm[_qp] * _coord[_qp]);
  else
    _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

void
ComputeWeightedGapLMMechanicalContact::computeQpIProperties()
{
  mooseAssert(_normals.size() ==
                  (_interpolate_normals ? _test[_i].size() : _lower_secondary_elem->n_nodes()),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const DofObject * dof = _var->isNodal()
                              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                              : static_cast<const DofObject *>(_lower_secondary_elem);

  if (_interpolate_normals)
    _dof_to_weighted_gap[dof].first += _test[_i][_qp] * _qp_gap;
  else
    _dof_to_weighted_gap[dof].first += _test[_i][_qp] * _qp_gap_nodal * _normals[_i];

  if (_normalize_c)
    _dof_to_weighted_gap[dof].second += _test[_i][_qp] * _qp_factor;
}

void
ComputeWeightedGapLMMechanicalContact::residualSetup()
{
  _dof_to_weighted_gap.clear();
}

void
ComputeWeightedGapLMMechanicalContact::jacobianSetup()
{
  residualSetup();
}

void
ComputeWeightedGapLMMechanicalContact::computeResidual(const Moose::MortarType mortar_type)
{
  if (mortar_type != Moose::MortarType::Lower)
    return;

  mooseAssert(_var, "LM variable is null");

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test.size(); ++_i)
      computeQpIProperties();
  }
}

void
ComputeWeightedGapLMMechanicalContact::computeJacobian(const Moose::MortarType mortar_type)
{
  // During "computeResidual" and "computeJacobian" we are actually just computing properties on the
  // mortar segment element mesh. We are *not* actually assembling into the residual/Jacobian. For
  // the zero-penetration constraint, the property of interest is the map from node to weighted gap.
  // Computation of the properties proceeds identically for residual and Jacobian evaluation hence
  // why we simply call computeResidual here. We will assemble into the residual/Jacobian later from
  // the post() method
  computeResidual(mortar_type);
}

void
ComputeWeightedGapLMMechanicalContact::post()
{
  parallel_object_only();

  const auto & dof_to_weighted_gap = _weighted_gap_uo.dofToWeightedGap();

  for (const auto & [dof_object, weighted_gap_pr] : dof_to_weighted_gap)
  {
    if (dof_object->processor_id() != this->processor_id())
      continue;

    const auto & [weighted_gap, normalization] = weighted_gap_pr;
    _weighted_gap_ptr = &weighted_gap;
    _normalization_ptr = &normalization;

    enforceConstraintOnDof(dof_object);
  }
}

void
ComputeWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  const auto & dof_to_weighted_gap = _weighted_gap_uo.dofToWeightedGap();

  for (const auto & [dof_object, weighted_gap_pr] : dof_to_weighted_gap)
  {
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof_object)) !=
         inactive_lm_nodes.end()) ||
        (dof_object->processor_id() != this->processor_id()))
      continue;

    const auto & [weighted_gap, normalization] = weighted_gap_pr;
    _weighted_gap_ptr = &weighted_gap;
    _normalization_ptr = &normalization;

    enforceConstraintOnDof(dof_object);
  }
}

void
ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const auto & weighted_gap = *_weighted_gap_ptr;
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;

  const auto dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  ADReal lm_value = (*_sys.currentSolution())(dof_index);
  Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);

  const ADReal dof_residual = std::min(lm_value, weighted_gap * c);

  _assembly.processResidualAndJacobian(dof_residual, dof_index, _vector_tags, _matrix_tags);
}
