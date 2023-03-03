//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedGapUserObject.h"
#include "MooseVariableField.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MortarContactUtils.h"
#include "libmesh/quadrature.h"

namespace
{
const InputParameters &
setBoundaryParam(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<std::vector<BoundaryName>>("boundary") = {
      params_in.get<BoundaryName>("secondary_boundary")};
  return ret;
}
}

InputParameters
WeightedGapUserObject::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<std::vector<BoundaryName>>("boundary");
  params.suppressParameter<std::vector<SubdomainName>>("block");
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
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

WeightedGapUserObject::WeightedGapUserObject(const InputParameters & parameters)
  : NodalUserObject(setBoundaryParam(parameters)),
    MortarConsumerInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _c(getParam<Real>("c")),
    _normalize_c(getParam<bool>("normalize_c")),
    _nodal(getVar("disp_x", 0)->feType().family == LAGRANGE),
    _disp_x_var(getVar("disp_x", 0)),
    _disp_y_var(getVar("disp_y", 0)),
    _has_disp_z(isCoupled("disp_z")),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
    _secondary_disp_x(_disp_x_var->adSln()),
    _primary_disp_x(_disp_x_var->adSlnNeighbor()),
    _secondary_disp_y(_disp_y_var->adSln()),
    _primary_disp_y(_disp_y_var->adSlnNeighbor()),
    _secondary_disp_z(_has_disp_z ? &_disp_z_var->adSln() : nullptr),
    _primary_disp_z(_has_disp_z ? &_disp_z_var->adSlnNeighbor() : nullptr),
    _coord(_assembly.mortarCoordTransformation())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("WeightedGapUserObject relies on use of the global indexing container "
             "in order to make its implementation feasible");
#endif

  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the WeightedGapUserObject object");
}

void
WeightedGapUserObject::initialSetup()
{
  std::array<const WeightedGapUserObject *, 1> consumers = {{this}};

  Moose::Mortar::setupMortarMaterials(consumers,
                                      _fe_problem,
                                      amg(),
                                      _tid,
                                      _secondary_ip_sub_to_mats,
                                      _primary_ip_sub_to_mats,
                                      _secondary_boundary_mats);

  _test = &test();
  _is_weighted_gap_nodal = isWeightedGapNodal();
  if (!_is_weighted_gap_nodal)
    mooseError("Currently inheriting from NodalUserObject so can't do elemental weighted gaps");
}

void
WeightedGapUserObject::computeQpProperties()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  // Trim interior node variable derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp{{_primary_disp_x[_mortar_qp],
                                      _primary_disp_y[_mortar_qp],
                                      _has_disp_z ? (*_primary_disp_z)[_mortar_qp] : 0}};
  std::array<ADReal, 3> secondary_disp{{_secondary_disp_x[_mortar_qp],
                                        _secondary_disp_y[_mortar_qp],
                                        _has_disp_z ? (*_secondary_disp_z)[_mortar_qp] : 0}};

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
  ADRealVectorValue gap_vec = _phys_points_primary[_mortar_qp] - _phys_points_secondary[_mortar_qp];

  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();
  if (_has_disp_z)
    gap_vec(2).derivatives() = prim_z->derivatives() - sec_z->derivatives();

  // Compute integration point quantities
  if (_interpolate_normals)
    _qp_gap = gap_vec * (_normals[_mortar_qp] * _JxW_msm[_mortar_qp] * _coord[_mortar_qp]);
  else
    _qp_gap_nodal = gap_vec * (_JxW_msm[_mortar_qp] * _coord[_mortar_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_mortar_qp] * _coord[_mortar_qp];
#endif
}

void
WeightedGapUserObject::computeQpIProperties()
{
  mooseAssert(_normals.size() ==
                  (_interpolate_normals ? (*_test)[_i].size() : _lower_secondary_elem->n_nodes()),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const DofObject * dof = _is_weighted_gap_nodal
                              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                              : static_cast<const DofObject *>(_lower_secondary_elem);

  if (_interpolate_normals)
    _dof_to_weighted_gap[dof].first += (*_test)[_i][_mortar_qp] * _qp_gap;
  else
    _dof_to_weighted_gap[dof].first += (*_test)[_i][_mortar_qp] * _qp_gap_nodal * _normals[_i];

  if (_normalize_c)
    _dof_to_weighted_gap[dof].second += (*_test)[_i][_mortar_qp] * _qp_factor;
}

void
WeightedGapUserObject::initialize()
{
  _dof_to_weighted_gap.clear();
}

void
WeightedGapUserObject::executeMortarSegment()
{
  for (_mortar_qp = 0; _mortar_qp < _qrule_msm->n_points(); _mortar_qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test->size(); ++_i)
      computeQpIProperties();
  }
}

void
WeightedGapUserObject::threadJoin(const UserObject & uo)
{
  const auto & weighted_gap_uo = static_cast<const WeightedGapUserObject &>(uo);
  _dof_to_weighted_gap.insert(weighted_gap_uo._dof_to_weighted_gap.begin(),
                              weighted_gap_uo._dof_to_weighted_gap.end());
}

void
WeightedGapUserObject::finalize()
{
  // There should be no need to communicate. We have the results for all local nodes
}

void
WeightedGapUserObject::execute()
{
  if (!hasDof(*_current_node))
    return;

  const auto & its = amg().secondariesToMortarSegments(*_current_node);

  auto act_functor = [this]()
  {
    setNormals();
    executeMortarSegment();
  };

  std::array<WeightedGapUserObject *, 1> consumers = {{this}};

  Moose::Mortar::loopOverMortarSegments(its,
                                        _assembly,
                                        _subproblem,
                                        _fe_problem,
                                        amg(),
                                        /*displaced=*/true,
                                        consumers,
                                        _tid,
                                        _secondary_ip_sub_to_mats,
                                        _primary_ip_sub_to_mats,
                                        _secondary_boundary_mats,
                                        act_functor);
}
