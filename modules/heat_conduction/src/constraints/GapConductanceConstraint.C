//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceConstraint.h"

registerADMooseObject("HeatConductionApp", GapConductanceConstraint);

defineADValidParams(
    GapConductanceConstraint,
    ADMortarConstraint,
    params.addClassDescription(
        "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
        "implementation of the thermal contact problem. For more information, see the "
        "detailed description here: http://tinyurl.com/gmmhbe9");

    params.addRequiredParam<Real>("k", "Gap conductance");
    params.addCoupledVar("displacements", "Displacement variables"));

template <ComputeStage compute_stage>
GapConductanceConstraint<compute_stage>::GapConductanceConstraint(
    const InputParameters & parameters)
  : ADMortarConstraint<compute_stage>(parameters),
    _k(adGetParam<Real>("k")),
    _disp_name(parameters.getVecMooseType("displacements")),
    _n_disp(_disp_name.size()),
    _disp_slave(_n_disp),
    _disp_master(_n_disp)

{
  for (unsigned int i = 0; i < _n_disp; ++i)
  {
    auto & disp_var = _subproblem.getStandardVariable(_tid, _disp_name[i]);
    _disp_slave[i] = &disp_var.template adSln<compute_stage>();
    _disp_master[i] = &disp_var.template adSlnNeighbor<compute_stage>();
  }
}

template <>
Real
GapConductanceConstraint<RESIDUAL>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Master:
      return _lambda[_qp] * _test_master[_i][_qp];

    case Moose::MortarType::Slave:
      return -_lambda[_qp] * _test_slave[_i][_qp];

    case Moose::MortarType::Lower:
    {
      // auto l = (_phys_points_master[_qp] - _phys_points_slave[_qp]).norm();
      // return (_k * (_u_master[_qp] - _u_slave[_qp]) / l - _lambda[_qp]) * _test[_i][_qp];
      return 0.0;
    }

    default:
      return 0;
  }
}

template <>
DualReal
GapConductanceConstraint<JACOBIAN>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Master:
      return _lambda[_qp] * _test_master[_i][_qp];

    case Moose::MortarType::Slave:
      return -_lambda[_qp] * _test_slave[_i][_qp];

    case Moose::MortarType::Lower:
    {
      return 0;

      // // we are creating a dual version of phys points master and slave here...
      // DualRealVectorValue dual_phys_points_master;
      // DualRealVectorValue dual_phys_points_slave;
      // for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      // {
      //   dual_phys_points_master(i) = _phys_points_master[_qp](i);
      //   dual_phys_points_slave(i) = _phys_points_slave[_qp](i);
      // }
      //
      // // ...which uses the derivative vector of the master and slave displacements as
      // // an approximation of the true phys points derivatives
      // for (unsigned int i = 0; i < _n_disp; ++i)
      // {
      //   dual_phys_points_master(i).derivatives() = (*_disp_master[i])[_qp].derivatives();
      //   dual_phys_points_slave(i).derivatives() = (*_disp_slave[i])[_qp].derivatives();
      // }
      //
      // auto l = (dual_phys_points_master - dual_phys_points_slave).norm();
      // return (_k * (_u_master[_qp] - _u_slave[_qp]) / l - _lambda[_qp]) * _test[_i][_qp];
    }

    default:
      return 0;
  }
}
