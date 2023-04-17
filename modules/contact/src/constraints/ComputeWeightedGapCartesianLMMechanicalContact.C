//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeWeightedGapCartesianLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "MortarContactUtils.h"
#include "Assembly.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

registerMooseObject("ContactApp", ComputeWeightedGapCartesianLMMechanicalContact);

namespace
{
const InputParameters &
assignVarsInParamsCartesianWeightedGap(const InputParameters & params_in)
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
ComputeWeightedGapCartesianLMMechanicalContact::validParams()
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
  params.addRequiredCoupledVar("lm_x",
                               "Mechanical contact Lagrange multiplier along the x Cartesian axis");
  params.addRequiredCoupledVar(
      "lm_y", "Mechanical contact Lagrange multiplier along the y Cartesian axis.");
  params.addCoupledVar("lm_z",
                       "Mechanical contact Lagrange multiplier along the z Cartesian axis.");
  params.set<bool>("interpolate_normals") = false;
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ComputeWeightedGapCartesianLMMechanicalContact::ComputeWeightedGapCartesianLMMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(assignVarsInParamsCartesianWeightedGap(parameters)),
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
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr)
{
  _lm_vars.push_back(getVar("lm_x", 0));
  _lm_vars.push_back(getVar("lm_y", 0));

  if (isParamValid("lm_z") ^ _has_disp_z)
    paramError("lm_z",
               "In three-dimensions, both the Z Lagrange multiplier and the Z displacement need to "
               "be provided");

  if (_has_disp_z)
    _lm_vars.push_back(getVar("lm_z", 0));

  if (!getParam<bool>("use_displaced_mesh"))
    paramError(
        "use_displaced_mesh",
        "'use_displaced_mesh' must be true for the ComputeWeightedGapLMMechanicalContact object");

  for (const auto i : index_range(_lm_vars))
    if (!_lm_vars[i]->isNodal())
      if (_lm_vars[i]->feType().order != static_cast<Order>(0))
        mooseError("Normal contact constraints only support elemental variables of CONSTANT order");
}

ADReal ComputeWeightedGapCartesianLMMechanicalContact::computeQpResidual(Moose::MortarType)
{
  mooseError("We should never call computeQpResidual for ComputeWeightedGapLMMechanicalContact");
}

void
ComputeWeightedGapCartesianLMMechanicalContact::computeQpProperties()
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
  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

void
ComputeWeightedGapCartesianLMMechanicalContact::computeQpIProperties()
{
  mooseAssert(_normals.size() == _lower_secondary_elem->n_nodes(),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const DofObject * dof = _lm_vars[0]->isNodal()
                              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                              : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_gap[dof].first += _test[_i][_qp] * _qp_gap_nodal * _normals[_i];

  if (_normalize_c)
    _dof_to_weighted_gap[dof].second += _test[_i][_qp] * _qp_factor;
}

void
ComputeWeightedGapCartesianLMMechanicalContact::timestepSetup()
{
  _dof_to_old_normal_vector.clear();

  for (auto & map_pr : _dof_to_normal_vector)
    _dof_to_old_normal_vector.emplace(map_pr);
}

void
ComputeWeightedGapCartesianLMMechanicalContact::residualSetup()
{
  _dof_to_weighted_gap.clear();
  _dof_to_normal_vector.clear();
  _dof_to_tangent_vectors.clear();
}

void
ComputeWeightedGapCartesianLMMechanicalContact::jacobianSetup()
{
  residualSetup();
}

void
ComputeWeightedGapCartesianLMMechanicalContact::computeResidual(const Moose::MortarType mortar_type)
{
  if (mortar_type != Moose::MortarType::Lower)
    return;

  for (const auto i : index_range(_lm_vars))
  {
    mooseAssert(_lm_vars[i], "LM variable is null");
    libmesh_ignore(i);
  }

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test.size(); ++_i)
    {
      computeQpIProperties();

      // Get the _dof_to_weighted_gap map
      const DofObject * dof =
          _lm_vars[0]->isNodal()
              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
              : static_cast<const DofObject *>(_lower_secondary_elem);
      // We do not interpolate geometry, so just match the local node _i with the corresponding _i
      _dof_to_normal_vector[dof] = _normals[_i];
      const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
      _dof_to_tangent_vectors[dof][0] = nodal_tangents[0][_i];
      _dof_to_tangent_vectors[dof][1] = nodal_tangents[1][_i];
    }
  }
}

void
ComputeWeightedGapCartesianLMMechanicalContact::computeJacobian(const Moose::MortarType mortar_type)
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
ComputeWeightedGapCartesianLMMechanicalContact::post()
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);

  for (const auto & pr : _dof_to_weighted_gap)
  {
    if (pr.first->processor_id() != this->processor_id())
      continue;

    _weighted_gap_ptr = &pr.second.first;
    _normalization_ptr = &pr.second.second;

    enforceConstraintOnDof(pr.first);
  }
}

void
ComputeWeightedGapCartesianLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);

  for (const auto & pr : _dof_to_weighted_gap)
  {
    if ((inactive_lm_nodes.find(static_cast<const Node *>(pr.first)) != inactive_lm_nodes.end()) ||
        (pr.first->processor_id() != this->processor_id()))
      continue;

    _weighted_gap_ptr = &pr.second.first;
    _normalization_ptr = &pr.second.second;

    enforceConstraintOnDof(pr.first);
  }
}

void
ComputeWeightedGapCartesianLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const auto & weighted_gap = *_weighted_gap_ptr;
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;

  const auto dof_index_x = dof->dof_number(_sys.number(), _lm_vars[0]->number(), 0);
  const auto dof_index_y = dof->dof_number(_sys.number(), _lm_vars[1]->number(), 0);

  ADReal lm_x = (*_sys.currentSolution())(dof_index_x);
  ADReal lm_y = (*_sys.currentSolution())(dof_index_y);

  Moose::derivInsert(lm_x.derivatives(), dof_index_x, 1.);
  Moose::derivInsert(lm_y.derivatives(), dof_index_y, 1.);

  dof_id_type dof_index_z(-1);
  ADReal lm_z;
  if (_has_disp_z)
  {
    dof_index_z = dof->dof_number(_sys.number(), _lm_vars[2]->number(), 0);
    lm_z = (*_sys.currentSolution())(dof_index_z);
    Moose::derivInsert(lm_z.derivatives(), dof_index_z, 1.);
  }

  ADReal normal_pressure_value =
      lm_x * _dof_to_normal_vector[dof](0) + lm_y * _dof_to_normal_vector[dof](1);
  ADReal tangential_pressure_value =
      lm_x * _dof_to_tangent_vectors[dof][0](0) + lm_y * _dof_to_tangent_vectors[dof][0](1);

  ADReal tangential_pressure_value_dir;

  if (_has_disp_z)
  {
    normal_pressure_value += lm_z * _dof_to_normal_vector[dof](2);
    tangential_pressure_value += lm_z * _dof_to_tangent_vectors[dof][0](2);
    tangential_pressure_value_dir = lm_x * _dof_to_tangent_vectors[dof][1](0) +
                                    lm_y * _dof_to_tangent_vectors[dof][1](1) +
                                    lm_z * _dof_to_tangent_vectors[dof][1](2);
  }

  ADReal normal_dof_residual = std::min(normal_pressure_value, weighted_gap * c);
  ADReal tangential_dof_residual = tangential_pressure_value;

  // Get index for normal constraint.
  // We do this to get a decent Jacobian structure, which is key for the use of iterative solvers.
  // Using old normal vector to avoid changes in the Jacobian structure within one time step

  Real ny, nz;
  // Intially, use the current normal vector
  if (_dof_to_old_normal_vector[dof].norm() < TOLERANCE)
  {
    ny = _dof_to_normal_vector[dof](1);
    nz = _dof_to_normal_vector[dof](2);
  }
  else
  {
    ny = _dof_to_old_normal_vector[dof](1);
    nz = _dof_to_old_normal_vector[dof](2);
  }
  unsigned int component_normal = 0;
  if (std::abs(ny) > 0.57735)
    component_normal = 1;
  else if (std::abs(nz) > 0.57735)
    component_normal = 2;

  libmesh_ignore(component_normal);

  _assembly.processResidualAndJacobian(
      normal_dof_residual,
      component_normal == 0 ? dof_index_x : (component_normal == 1 ? dof_index_y : dof_index_z),
      _vector_tags,
      _matrix_tags);

  _assembly.processResidualAndJacobian(
      tangential_dof_residual,
      (component_normal == 0 || component_normal == 2) ? dof_index_y : dof_index_x,
      _vector_tags,
      _matrix_tags);

  if (_has_disp_z)
    _assembly.processResidualAndJacobian(
        tangential_pressure_value_dir,
        (component_normal == 0 || component_normal == 1) ? dof_index_z : dof_index_x,
        _vector_tags,
        _matrix_tags);
}
