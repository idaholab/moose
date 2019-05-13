//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialContactLMTest.h"

registerADMooseObject("MooseTestApp", TangentialContactLMTest);

defineADValidParams(
    TangentialContactLMTest,
    ADMortarConstraint,
    params.addRequiredParam<NonlinearVariableName>("slave_disp_y",
                                                   "The y displacement variable on the slave face");
    params.addParam<NonlinearVariableName>("master_disp_y",
                                           "The y displacement variable on the master face");
    params.addRequiredParam<NonlinearVariableName>(
        "contact_pressure",
        "The normal contact pressure; oftentimes this may be a separate lagrange multiplier "
        "variable");
    params.addRequiredParam<Real>("friction_coefficient", "The friction coefficient"););

template <ComputeStage compute_stage>
TangentialContactLMTest<compute_stage>::TangentialContactLMTest(const InputParameters & parameters)
  : ADMortarConstraint<compute_stage>(parameters),
    _slave_disp_y(_subproblem.getStandardVariable(_tid, parameters.getMooseType("slave_disp_y"))),
    _master_disp_y(
        isParamValid("master_disp_y")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_disp_y"))
            : _subproblem.getStandardVariable(_tid, parameters.getMooseType("slave_disp_y"))),
    _contact_pressure_var(
        _subproblem.getStandardVariable(_tid, parameters.getMooseType("contact_pressure"))),
    _contact_pressure(_contact_pressure_var.template adSlnLower<compute_stage>()),
    _slave_x_dot(_slave_var.template adUDot<compute_stage>()),
    _master_x_dot(_master_var.template adUDotNeighbor<compute_stage>()),
    _slave_y_dot(_slave_disp_y.template adUDot<compute_stage>()),
    _master_y_dot(_master_disp_y.template adUDotNeighbor<compute_stage>()),
    _friction_coeff(adGetParam<Real>("friction_coefficient"))
{
}

template <ComputeStage compute_stage>
ADReal
TangentialContactLMTest<compute_stage>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      // Check whether we project onto a master face
      if (_has_master)
      {
        // Check whether we are actually in contact
        if (_contact_pressure[_qp] > TOLERANCE)
        {
          // Build the velocity vector
          ADRealVectorValue relative_velocity(
              _slave_x_dot[_qp] - _master_x_dot[_qp], _slave_y_dot[_qp] - _master_y_dot[_qp], 0);

          // Get the component in the tangential direction
          auto tangential_velocity = relative_velocity * _tangents[_qp][0];

          // NCP part 1: requirement that either there is no slip **or** slip velocity and
          // frictional force exerted **by** the slave side are in the same direction
          auto a = tangential_velocity / _lambda[_qp];

          // NCP part 2: require that the frictional force can never exceed the frictional
          // coefficient times the normal force
          auto b = _friction_coeff * _contact_pressure[_qp] - std::abs(_lambda[_qp]);

          return a + b - std::sqrt(a * a + b * b);
        }
        else
          // If not in contact then we force the tangential lagrange multiplier to zero
          return _test[_i][_qp] * _lambda[_qp];
      }
      else
        // If not in contact then we force the tangential lagrange multiplier to zero (if we don't
        // project onto a master face, then we're definitely not in contact)
        return _test[_i][_qp] * _lambda[_qp];
    }

    default:
      return 0;
  }
}
