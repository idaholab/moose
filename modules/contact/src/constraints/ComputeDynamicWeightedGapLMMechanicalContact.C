//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDynamicWeightedGapLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

registerMooseObject("ContactApp", ComputeDynamicWeightedGapLMMechanicalContact);

namespace
{
}

InputParameters
ComputeDynamicWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addRangeCheckedParam<Real>(
      "newmark_beta", "newmark_beta>0.0", "beta parameter for Newmark Time integration");
  params.addClassDescription("Computes the weighted gap that will later be used to enforce the "
                             "zero-penetration mechanical contact conditions");
  return params;
}

ComputeDynamicWeightedGapLMMechanicalContact::ComputeDynamicWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
    _secondary_x_dot(adCoupledDot("disp_x")),
    _primary_x_dot(adCoupledNeighborValueDot("disp_x")),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_x_dotdot(adCoupledDotDot("disp_x")),
    _primary_x_dotdot(adCoupledNeighborValueDotDot("disp_x")),
    _secondary_y_dotdot(adCoupledDotDot("disp_y")),
    _primary_y_dotdot(adCoupledNeighborValueDotDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _secondary_z_dotdot(_has_disp_z ? &adCoupledDotDot("disp_z") : nullptr),
    _primary_z_dotdot(_has_disp_z ? &adCoupledNeighborValueDotDot("disp_z") : nullptr),
    _has_beta(isParamValid("newmark_beta")),
    _beta(_has_beta ? getParam<Real>("newmark_beta") : 0.1)
{
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeQpProperties()
{
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  ADRealVectorValue relative_velocity(_secondary_x_dot[_qp] - _primary_x_dot[_qp],
                                      _secondary_y_dot[_qp] - _primary_y_dot[_qp],
                                      0.0);
  ADRealVectorValue relative_acc(_secondary_x_dotdot[_qp] - _primary_x_dotdot[_qp],
                                 _secondary_y_dotdot[_qp] - _primary_y_dotdot[_qp],
                                 0.0);

  if (_has_disp_z)
  {
    relative_velocity(2) = (*_secondary_z_dot)[_qp] - (*_primary_z_dot)[_qp];
    relative_acc(2) = (*_secondary_z_dotdot)[_qp] - (*_primary_z_dotdot)[_qp];
  }

  gap_vec(0).derivatives() =
      _primary_disp_x[_qp].derivatives() - _secondary_disp_x[_qp].derivatives();
  gap_vec(1).derivatives() =
      _primary_disp_y[_qp].derivatives() - _secondary_disp_y[_qp].derivatives();

  if (_has_disp_z)
    gap_vec(2).derivatives() =
        (*_primary_disp_z)[_qp].derivatives() - (*_secondary_disp_z)[_qp].derivatives();

  if (_interpolate_normals)
    _qp_gap = (gap_vec - 0.5 * (relative_velocity * _dt + _beta * relative_acc * _dt * _dt)) *
              (_normals[_qp] * _JxW_msm[_qp] * _coord[_qp]);
  else
    _qp_gap_nodal = (gap_vec - 0.5 * (relative_velocity * _dt + _beta * relative_acc * _dt * _dt)) *
                    (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}
