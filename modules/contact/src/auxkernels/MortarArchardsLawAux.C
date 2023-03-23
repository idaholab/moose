//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarArchardsLawAux.h"
#include "MooseVariableFE.h"
#include "FEProblemBase.h"
#include "Assembly.h"

registerMooseObject("ContactApp", MortarArchardsLawAux);

InputParameters
MortarArchardsLawAux::validParams()
{
  InputParameters params = MortarNodalAuxKernel::validParams();
  params.addClassDescription(
      "Returns the weighted gap velocity at a node. This quantity is useful for mortar contact, "
      "particularly when dual basis functions are used in contact mechanics");
  params.addCoupledVar("v",
                       "Optional variable to take the value of. If omitted the value of the "
                       "`variable` itself is returned.");
  params.addRequiredCoupledVar("displacements",
                               "The displacement variables. This mortar nodal auxiliary kernel can "
                               "take two or three displacements");
  params.addRequiredCoupledVar("normal_pressure",
                               "The name of the Lagrange multiplier that holds the normal contact "
                               "pressure in mortar formulations");
  params.addParam<bool>("use_displaced_mesh",
                        true,
                        "Whether to use the displaced mesh to compute the auxiliary kernel value");
  params.addParam<bool>(
      "incremental",
      true,
      "Whether to accumulate worn-out depth (The default 'true' is strongly recommended)");
  params.addRequiredParam<Real>(
      "friction_coefficient",
      "Friction coefficient used to compute wear (to match that of frictional contact)");
  params.addRequiredParam<Real>(
      "energy_wear_coefficient",
      "Energy wear coefficient is a surface-dependent parameter used in Archard's wear law");
  params.set<bool>("interpolate_normals") = false;

  return params;
}

MortarArchardsLawAux::MortarArchardsLawAux(const InputParameters & parameters)
  : MortarNodalAuxKernel(parameters),
    _normal_pressure(coupledValueLower("normal_pressure")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _energy_wear_coefficient(getParam<Real>("energy_wear_coefficient")),
    _displacements(
        {getVar("displacements", 0), getVar("displacements", 1), getVar("displacements", 2)}),
    _has_disp_z(_displacements[2] ? true : false),
    _secondary_x_dot(_displacements[0]->adUDot()),
    _primary_x_dot(_displacements[0]->adUDotNeighbor()),
    _secondary_y_dot(_displacements[1]->adUDot()),
    _primary_y_dot(_displacements[1]->adUDotNeighbor()),
    _secondary_z_dot(_has_disp_z ? &_displacements[2]->adUDot() : nullptr),
    _primary_z_dot(_has_disp_z ? &_displacements[2]->adUDotNeighbor() : nullptr),
    _worn_depth(0),
    _qp_gap_velocity_nodal(0)
{
  if (!_displaced)
    paramError("use_displaced_mesh",
               "The MortarArchardsLawAux auxiliary kernel requires the use of displaced meshes to "
               "compute the worn-out depth.");
}

Real
MortarArchardsLawAux::computeValue()
{
  mooseAssert(_normals.size() == _test_lower.size(),
              "Normals and test_lower must be the same size");
  _worn_depth = 0;
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test_lower.size(); ++_i)
      computeQpIProperties();
  }

  return _worn_depth;
}

void
MortarArchardsLawAux::computeQpProperties()
{
  _msm_volume += _JxW_msm[_qp] * _coord_msm[_qp];
}

void
MortarArchardsLawAux::computeQpIProperties()
{
  RealVectorValue gap_velocity_vec;
  gap_velocity_vec(0) = MetaPhysicL::raw_value(_secondary_x_dot[_qp] - _primary_x_dot[_qp]);
  gap_velocity_vec(1) = MetaPhysicL::raw_value(_secondary_y_dot[_qp] - _primary_y_dot[_qp]);

  if (_has_disp_z)
    gap_velocity_vec(2) = MetaPhysicL::raw_value((*_secondary_z_dot)[_qp] - (*_primary_z_dot)[_qp]);

  // Remove point-wise normal component of the relative velocity
  gap_velocity_vec -= gap_velocity_vec.contract(_normals[_i]) * _normals[_i];

  // Compute norm of the relative tangential velocity (used to compute the weighted quantity)
  const auto norm_tangential_vel = gap_velocity_vec.norm();

  const auto worn_out_depth_dt = norm_tangential_vel * _energy_wear_coefficient *
                                 _friction_coefficient * _normal_pressure[0] * _dt * _JxW_msm[_qp] *
                                 _coord_msm[_qp];

  // Accumulate worn-out depth over time.
  _worn_depth += _test_lower[_i][_qp] * worn_out_depth_dt;
}
