//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedGapVelAux.h"
#include "MooseVariableFE.h"
#include "FEProblemBase.h"
#include "Assembly.h"

registerMooseObject("ContactApp", WeightedGapVelAux);

InputParameters
WeightedGapVelAux::validParams()
{
  InputParameters params = MortarNodalAuxKernel::validParams();
  params.addClassDescription(
      "Returns the weighted gap velocity at a node. This quantity is useful for mortar contact, "
      "particularly when dual basis functions are used in contact mechanics");
  params.addCoupledVar("v",
                       "Optional variable to take the value of. If omitted the value of the "
                       "`variable` itself is returned.");
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.set<bool>("interpolate_normals") = false;
  params.addParam<bool>("use_displaced_mesh",
                        true,
                        "Whether to use the displaced mesh to compute the auxiliary kernel value.");
  return params;
}

WeightedGapVelAux::WeightedGapVelAux(const InputParameters & parameters)
  : MortarNodalAuxKernel(parameters),
    _has_disp_z(isCoupled("disp_z")),
    _disp_x(*getVar("disp_x", 0)),
    _disp_y(*getVar("disp_y", 0)),
    _disp_z(getVar("disp_z", 0)),
    _secondary_x_dot(_disp_x.adUDot()),
    _primary_x_dot(_disp_x.adUDotNeighbor()),
    _secondary_y_dot(_disp_y.adUDot()),
    _primary_y_dot(_disp_y.adUDotNeighbor()),
    _secondary_z_dot(_has_disp_z ? &_disp_z->adUDot() : nullptr),
    _primary_z_dot(_has_disp_z ? &_disp_z->adUDotNeighbor() : nullptr),
    _weighted_gap_velocity(0),
    _qp_gap_velocity(0),
    _qp_gap_velocity_nodal(0)
{
  if (!_displaced)
    paramWarning(
        "use_displaced_mesh",
        "This auxiliary kernel typically requires the use of displaced meshes to compute the "
        "weighted gap velocity.");
}

Real
WeightedGapVelAux::computeValue()
{
  _weighted_gap_velocity = 0;
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test_lower.size(); ++_i)
      computeQpIProperties();
  }

  return _weighted_gap_velocity;
}

void
WeightedGapVelAux::computeQpProperties()
{
  RealVectorValue gap_velocity_vec;
  gap_velocity_vec(0) = MetaPhysicL::raw_value(_secondary_x_dot[_qp] - _primary_x_dot[_qp]);
  gap_velocity_vec(1) = MetaPhysicL::raw_value(_secondary_y_dot[_qp] - _primary_y_dot[_qp]);

  if (_has_disp_z)
    gap_velocity_vec(2) = MetaPhysicL::raw_value((*_secondary_z_dot)[_qp] - (*_primary_z_dot)[_qp]);

  _qp_gap_velocity_nodal = gap_velocity_vec * (_JxW_msm[_qp] * _coord_msm[_qp]);

  _msm_volume += _JxW_msm[_qp] * _coord_msm[_qp];
}

void
WeightedGapVelAux::computeQpIProperties()
{
  _weighted_gap_velocity += _test_lower[_i][_qp] * _qp_gap_velocity_nodal * _normals[_i];
}
