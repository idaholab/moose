//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedGapUserObject.h"
#include "MooseVariableField.h"
#include "SubProblem.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MortarContactUtils.h"
#include "AutomaticMortarGeneration.h"
#include "ADUtils.h"

#include "libmesh/quadrature.h"

#include <limits>

InputParameters
WeightedGapUserObject::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<bool>("interpolate_normals") = false;
  params.addPrivateParam<bool>("allow_nodal_normal_derivatives", false);
  params.addPrivateParam<bool>("use_nodal_normal_derivatives", false);
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

WeightedGapUserObject::WeightedGapUserObject(const InputParameters & parameters)
  : MortarUserObject(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
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
    _coord(_assembly.mortarCoordTransformation()),
    _allow_nodal_normal_derivatives(getParam<bool>("allow_nodal_normal_derivatives")),
    _use_nodal_normal_derivatives(false)
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the WeightedGapUserObject object");

  // A valid penetration_tolerance identifies augmented-Lagrange contact, which retains frozen
  // normal directions.
  if (getParam<bool>("use_nodal_normal_derivatives") && !isParamValid("penetration_tolerance"))
    includeNodalNormalDerivatives();
}

void
WeightedGapUserObject::includeNodalNormalDerivatives()
{
  if (_use_nodal_normal_derivatives)
    return;

  if (!_allow_nodal_normal_derivatives)
    mooseError("Nodal-normal derivatives are not supported by user object '", name(), "'.");

  if (getParam<bool>("interpolate_normals"))
    paramError("interpolate_normals",
               "Nodal-normal derivatives require normalized secondary nodal normals and cannot be "
               "combined with quadrature-point normal interpolation.");

  const std::array<std::pair<const MooseVariable *, const char *>, 3> displacement_variables{
      {{_disp_x_var, "disp_x"}, {_disp_y_var, "disp_y"}, {_disp_z_var, "disp_z"}}};
  for (const auto & [variable, parameter_name] : displacement_variables)
    if (variable)
    {
      if (!variable->isNodal())
        paramError(parameter_name,
                   "Nodal-normal derivatives require a nodal displacement variable.");
      if (&variable->sys() != &_sys)
        paramError(parameter_name,
                   "Nodal-normal derivatives require displacement variables in the nonlinear "
                   "system assembled by this contact object.");
    }

  _use_nodal_normal_derivatives = true;
}

bool
WeightedGapUserObject::usesNodalNormalDerivatives() const
{
  return _use_nodal_normal_derivatives && Moose::doDerivatives(_subproblem, _sys);
}

void
WeightedGapUserObject::initialSetup()
{
  MortarUserObject::initialSetup();
  _test = &test();
}

void
WeightedGapUserObject::computeQpProperties()
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

  // Generic displacement for interface problems
  _qp_displacement_nodal(0) = prim_x - sec_x;
  _qp_displacement_nodal(1) = prim_y - sec_y;
  if (_has_disp_z)
    _qp_displacement_nodal(2) = *prim_z - *sec_z;

  _qp_displacement_nodal *= _JxW_msm[_qp] * _coord[_qp];

  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();
  if (_has_disp_z)
    gap_vec(2).derivatives() = prim_z->derivatives() - sec_z->derivatives();

  // Compute integration point quantities: Normals (geometry) is averaged at the node, but not
  // interpolated within the weak integration.
  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

void
WeightedGapUserObject::computeQpIProperties()
{
  if (usesNodalNormalDerivatives())
    mooseAssert(_lower_secondary_elem->n_nodes() > _i,
                "Making sure that the nodal normal index is valid");
  else
    mooseAssert(_normals.size() > _i, "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  auto & [weighted_gap, normalization] = _dof_to_weighted_gap[dof];

  if (usesNodalNormalDerivatives())
    weighted_gap += (*_test)[_i][_qp] * _qp_gap_nodal * contactNormal(*_lower_secondary_elem, _i);
  else
    weighted_gap += (*_test)[_i][_qp] * _qp_gap_nodal * _normals[_i];

  normalization += (*_test)[_i][_qp] * _qp_factor;

  _dof_to_weighted_displacements[dof] += (*_test)[_i][_qp] * _qp_displacement_nodal;
}

const ADRealVectorValue &
WeightedGapUserObject::contactNormal(const Elem & lower_secondary_elem,
                                     const unsigned int nodal_index) const
{
  mooseAssert(nodal_index < lower_secondary_elem.n_nodes(),
              "Nodal normal index must refer to a node on the secondary element.");
  mooseAssert(usesNodalNormalDerivatives(),
              "AD contact normals should only be requested while recording nodal-normal "
              "derivatives.");
  const Node * const node = lower_secondary_elem.node_ptr(nodal_index);
  const auto normal_it = _ad_nodal_normals.find(node);
  if (normal_it != _ad_nodal_normals.end())
    return normal_it->second;

  amg().computeADNodalNormals(
      [this](const Node & coordinate_node, const Point & geometry_coordinate)
      { return nodalCoordinate(coordinate_node, geometry_coordinate); },
      _ad_nodal_normals);
  return libmesh_map_find(_ad_nodal_normals, node);
}

ADPoint
WeightedGapUserObject::nodalCoordinate(const Node & node, const Point & geometry_coordinate) const
{
  mooseAssert(usesNodalNormalDerivatives(),
              "AD nodal coordinates should only be requested while recording nodal-normal "
              "derivatives.");
  ADPoint point = geometry_coordinate;

  const std::array<std::pair<const MooseVariable *, unsigned int>, 3> displacement_variables{
      {{_disp_x_var, 0}, {_disp_y_var, 1}, {_disp_z_var, 2}}};
  for (const auto & [variable, component] : displacement_variables)
    if (variable)
    {
      const auto sys_num = variable->sys().number();
      const auto var_num = variable->number();
      if (node.n_dofs(sys_num, var_num))
        Moose::derivInsert(
            point(component).derivatives(), node.dof_number(sys_num, var_num, 0), 1.0);
    }

  return point;
}

void
WeightedGapUserObject::initialize()
{
  _ad_nodal_normals.clear();
  _dof_to_weighted_gap.clear();
  _dof_to_weighted_displacements.clear();
}

void
WeightedGapUserObject::finalize()
{
  // If the constraint is performed by the owner, then we don't need any data sent back; the owner
  // will take care of it. But if the constraint is not performed by the owner and we might have to
  // do some of the constraining ourselves, then we need data sent back to us
  const bool send_data_back = !constrainedByOwner();
  Moose::Mortar::Contact::communicateGaps(_dof_to_weighted_gap,
                                          _subproblem.mesh(),
                                          _nodal,
                                          /*normalize_c*/ true,
                                          _communicator,
                                          send_data_back);
}

void
WeightedGapUserObject::execute()
{
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test->size(); ++_i)
      computeQpIProperties();
  }
}

Real
WeightedGapUserObject::getNormalGap(const Node * const node) const
{
  const auto it = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  // We are returning the physical weighted gap for analysis purposes
  if (it != _dof_to_weighted_gap.end())
    return physicalGap(it->second);
  else
    return 0.0;
}
