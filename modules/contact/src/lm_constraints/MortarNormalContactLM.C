//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarNormalContactLM.h"

registerADMooseObject("ContactApp", MortarNormalContactLM);

defineADValidParams(
    MortarNormalContactLM,
    ADMortarConstraint,
    params.addParam<NonlinearVariableName>("slave_disp_y",
                                           "The y displacement variable on the slave face");
    params.addParam<NonlinearVariableName>("master_disp_y",
                                           "The y displacement variable on the master face"););

template <ComputeStage compute_stage>
MortarNormalContactLM<compute_stage>::MortarNormalContactLM(const InputParameters & parameters)
  : ADMortarConstraint<compute_stage>(parameters),
    _slave_disp_y(isParamValid("slave_disp_y") ? &_subproblem.getStandardVariable(
                                                     _tid, parameters.getMooseType("slave_disp_y"))
                                               : nullptr),
    _master_disp_y(
        isParamValid("master_disp_y")
            ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("master_disp_y"))
            : isParamValid("slave_disp_y")
                  ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("slave_disp_y"))
                  : nullptr),
    _computing_gap_dependence(false),
    _slave_disp_y_sln(nullptr),
    _master_disp_y_sln(nullptr),
    _epsilon(std::numeric_limits<Real>::epsilon())
{
  if (_slave_disp_y)
  {
    mooseAssert(_master_disp_y,
                "It doesn't make any sense that we have a slave displacement variable and not a "
                "master displacement variable");
    _computing_gap_dependence = true;
    _slave_disp_y_sln = &_slave_disp_y->template adSln<compute_stage>();
    _master_disp_y_sln = &_master_disp_y->template adSlnNeighbor<compute_stage>();
  }
}

template <>
Real
MortarNormalContactLM<RESIDUAL>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      if (_has_master)
      {
        auto gap_vec = _phys_points_master[_qp] - _phys_points_slave[_qp];
        auto gap = gap_vec * _normals[_qp];

        // The FB function (in its pure form) is not differentiable at (0, 0) but if we add some
        // constant > 0 into the root function, then it is
        auto fb_function =
            _lambda[_qp] + gap - std::sqrt(_lambda[_qp] * _lambda[_qp] + gap * gap + _epsilon);

        return _test[_i][_qp] * fb_function;
      }
      else
        return _test[_i][_qp] * _lambda[_qp];
    }

    default:
      return 0;
  }
}

template <>
DualReal
MortarNormalContactLM<JACOBIAN>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      if (_has_master)
      {
        DualRealVectorValue gap_vec = _phys_points_master[_qp] - _phys_points_slave[_qp];
        if (_computing_gap_dependence)
        {
          // Here we're assuming that the user provided the x-component as the slave/master
          // variable!
          gap_vec(0).derivatives() = _u_master[_qp].derivatives() - _u_slave[_qp].derivatives();
          gap_vec(1).derivatives() =
              (*_master_disp_y_sln)[_qp].derivatives() - (*_slave_disp_y_sln)[_qp].derivatives();
        }

        auto gap = gap_vec * _normals[_qp];

        // The FB function (in its pure form) is not differentiable at (0, 0) but if we add some
        // constant > 0 into the root function, then it is
        auto fb_function =
            _lambda[_qp] + gap - std::sqrt(_lambda[_qp] * _lambda[_qp] + gap * gap + _epsilon);

        return _test[_i][_qp] * fb_function;
      }
      else
        return _test[_i][_qp] * _lambda[_qp];
    }

    default:
      return 0;
  }
}
