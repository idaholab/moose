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
  params.set<bool>("use_displaced_mesh") = true;
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
    _normal_index(_interpolate_normals ? _qp : _i),
    _c(getParam<Real>("c"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("ComputeWeightedGapLMMechanicalContact relies on use of the global indexing container "
             "in order to make its implementation feasible");
#endif

  if (!getParam<bool>("use_displaced_mesh"))
    paramError(
        "use_displaced_mesh",
        "'use_displaced_mesh' must be true for the ComputeWeightedGapLMMechanicalContact object");
}

ADReal ComputeWeightedGapLMMechanicalContact::computeQpResidual(Moose::MortarType)
{
  mooseError("We should never call computeQpResidual for ComputeWeightedGapLMMechanicalContact");
}

void
ComputeWeightedGapLMMechanicalContact::computeQpProperties()
{
  if (_has_primary)
  {
    ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

    gap_vec(0).derivatives() =
        _primary_disp_x[_qp].derivatives() - _secondary_disp_x[_qp].derivatives();
    gap_vec(1).derivatives() =
        _primary_disp_y[_qp].derivatives() - _secondary_disp_y[_qp].derivatives();
    if (_has_disp_z)
      gap_vec(2).derivatives() =
          (*_primary_disp_z)[_qp].derivatives() - (*_secondary_disp_z)[_qp].derivatives();

    _qp_gap = gap_vec * (_normals[_normal_index] * _JxW_msm[_qp] * _coord[_qp]);
  }
  else
    _qp_gap = std::numeric_limits<Real>::quiet_NaN();
}

void
ComputeWeightedGapLMMechanicalContact::computeQpIProperties()
{
  mooseAssert(_normals.size() ==
                  (_interpolate_normals ? _test[_i].size() : _lower_secondary_elem->n_nodes()),
              "Making sure that _normals is the expected size");

  // If elemental, make sure CONSTANT MONOMIAL; need to figure out how to index dofs
  // to enable higher order elemental
  if (!_var->isNodal())
    mooseAssert(_i < 1, "Elemental variables must be CONSTANT order.");

  // Get the _dof_to_weighted_gap map
  const DofObject * const dof = _var->isNodal() ?
                                static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i)) :
                                static_cast<const DofObject *>(_lower_secondary_elem);
  _dof_to_weighted_gap[dof].first += _test[_i][_qp] * _qp_gap;


  // For non-dual contact assemble weighted traction (lambda represents traction)
  if (!_var->useDual())
  {
    _dof_to_weighted_gap[dof].second += _test[_i][_qp] * _lambda[_qp];
  }
  // For dual, on first quadrature point evaluation store nodal value in place of weighted traction
  else if (/*_var->useDual() &&*/ _qp == 0)
  {
    const auto * const node = _lower_secondary_elem->node_ptr(_i);
    const auto dof_index = node->dof_number(_sys.number(), _var->number(), 0);

    ADReal lm_value = _var->getNodalValue(*node);
    Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);
    _dof_to_weighted_gap[node].second = lm_value;
  }
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
  for (const auto & pr : _dof_to_weighted_gap)
  {
    _weighted_gap_ptr = &pr.second.first;
    _weighted_traction_ptr = &pr.second.second;

    enforceConstraintOnDof(pr.first);
  }
}

void
ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const auto dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const auto & weighted_gap = *_weighted_gap_ptr;
  const auto & weighted_traction = *_weighted_traction_ptr;
  const ADReal dof_residual =
      std::isnan(weighted_gap) ? weighted_traction : std::min(weighted_traction, weighted_gap * _c);

  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(dof_residual, dof_index, _matrix_tags);
  else
    _assembly.cacheResidual(dof_index, dof_residual.value(), _vector_tags);
}
