//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "NonlinearSystemBase.h"
#include "metaphysicl/metaphysicl_version.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#if METAPHYSICL_MAJOR_VERSION < 2
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#else
#include "metaphysicl/parallel_dynamic_array_wrapper.h"
#endif
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

#include <cmath>

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
  params.addParam<bool>("use_derived_c_normal",
                        false,
                        "Read c_normal per-node from the weighted gap UO's dofToDerivedC() map "
                        "instead of using the scalar 'c' parameter.");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<bool>("interpolate_normals") = false;
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
    _use_derived_c_normal(getParam<bool>("use_derived_c_normal")),
    _normalize_c(getParam<bool>("normalize_c")),
    _nodal(getVar("disp_x", 0)->feType().family == LAGRANGE),
    _disp_x_var(getVar("disp_x", 0)),
    _disp_y_var(getVar("disp_y", 0)),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
    _weighted_gap_uo(getUserObject<LMWeightedGapUserObject>("weighted_gap_uo"))
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError(
        "use_displaced_mesh",
        "'use_displaced_mesh' must be true for the ComputeWeightedGapLMMechanicalContact object");

  if (!_var->isNodal())
    if (_var->feType().order != static_cast<Order>(0))
      mooseError("Normal contact constraints only support elemental variables of CONSTANT order");

  if (!_use_derived_c_normal && (!std::isfinite(_c) || _c <= 0.0))
    paramError("c", "The user-supplied normal contact pressure scale must be positive and finite.");

  _fe_problem.getNonlinearSystemBase(_sys.number()).requestKSPRightDiagonalScale();
}

ADReal
ComputeWeightedGapLMMechanicalContact::computeQpResidual(Moose::MortarType)
{
  mooseError("We should never call computeQpResidual for ComputeWeightedGapLMMechanicalContact");
}

void
ComputeWeightedGapLMMechanicalContact::computeQpProperties()
{
}

void
ComputeWeightedGapLMMechanicalContact::computeQpIProperties()
{
}

void
ComputeWeightedGapLMMechanicalContact::residualSetup()
{
}

void
ComputeWeightedGapLMMechanicalContact::jacobianSetup()
{
}

void
ComputeWeightedGapLMMechanicalContact::computeResidual(const Moose::MortarType /*mortar_type*/)
{
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

  _fe_problem.getNonlinearSystemBase(_sys.number()).closeKSPRightDiagonalScale();
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

  _fe_problem.getNonlinearSystemBase(_sys.number()).closeKSPRightDiagonalScale();
}

Real
ComputeWeightedGapLMMechanicalContact::displacementScaling() const
{
  const std::array<Real, 3> scalings = {_disp_x_var->scalingFactor(),
                                        _disp_y_var->scalingFactor(),
                                        _disp_z_var ? _disp_z_var->scalingFactor() : 1.0};
  const auto num_displacements = _disp_z_var ? 3 : 2;
  Real log_sum = 0.0;
  for (const auto i : make_range(num_displacements))
  {
    if (!std::isfinite(scalings[i]) || scalings[i] <= 0.0)
      mooseError("Mortar contact scaling requires positive, finite displacement variable scaling "
                 "factors.");
    log_sum += std::log(scalings[i]);
  }

  if (MooseUtils::relativeFuzzyEqual(scalings[0], scalings[1], 1e-12) &&
      (!_disp_z_var || MooseUtils::relativeFuzzyEqual(scalings[0], scalings[2], 1e-12)))
    return scalings[0];

  // A contact pressure contributes to every displacement component. Use one symmetric
  // characteristic scale for its multiplier equation when component scalings differ.
  return std::exp(log_sum / num_displacements);
}

Real
ComputeWeightedGapLMMechanicalContact::equationCompensation(const MooseVariable & multiplier) const
{
  const Real multiplier_scaling = multiplier.scalingFactor();
  if (!std::isfinite(multiplier_scaling) || multiplier_scaling <= 0.0)
    mooseError("Mortar contact scaling requires positive, finite Lagrange multiplier scaling "
               "factors.");
  return displacementScaling() / multiplier_scaling;
}

Real
ComputeWeightedGapLMMechanicalContact::contactNormalization() const
{
  const Real normalization = *_normalization_ptr;
  if (!std::isfinite(normalization) || normalization <= 0.0)
    mooseError("Mortar contact requires positive, finite nodal mortar weights.");
  return normalization;
}

Real
ComputeWeightedGapLMMechanicalContact::normalContactScale(const DofObject * const dof) const
{
  const Real scale = _use_derived_c_normal
                         ? libmesh_map_find(_weighted_gap_uo.dofToDerivedC(), dof)[0]
                         : _c * (_normalize_c ? 1.0 : contactNormalization());
  if (!std::isfinite(scale) || scale <= 0.0)
    mooseError("Mortar contact requires positive, finite nodal normal pressure scales.");
  return scale;
}

void
ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const auto & weighted_gap = *_weighted_gap_ptr;

  const Real normal_scale = normalContactScale(dof);
  const Real c = normal_scale / contactNormalization();

  const auto dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  ADReal lm_value = (*_sys.currentSolution())(dof_index);
  Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);

  _fe_problem.getNonlinearSystemBase(_sys.number())
      .setKSPRightDiagonalScale(dof_index, normal_scale);

  const ADReal dof_residual = equationCompensation(*_var) * std::min(lm_value, weighted_gap * c);

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{dof_residual}},
                          std::array<dof_id_type, 1>{{dof_index}},
                          _var->scalingFactor());
}
