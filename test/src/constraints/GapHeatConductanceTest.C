//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatConductanceTest.h"

registerADMooseObject("MooseTestApp", GapHeatConductanceTest);

defineADValidParams(
    GapHeatConductanceTest,
    RealMortarConstraint,
    params.addParam<Real>("gap_conductance_constant",
                          0.03,
                          "The numerator in calculation of the heat transfer coefficient"););

template <ComputeStage compute_stage>
GapHeatConductanceTest<compute_stage>::GapHeatConductanceTest(const InputParameters & parameters)
  : RealMortarConstraint<compute_stage>(parameters),
    _gap_conductance_constant(adGetParam<Real>("gap_conductance_constant"))
{
}

template <ComputeStage compute_stage>
ADResidual
GapHeatConductanceTest<compute_stage>::computeLMQpResidual()
{
  Real heat_transfer_coeff(0);
  if (_has_master)
  {
    auto gap = (_xyz_slave_interior[_qp] - _xyz_master_interior[_qp]).norm();
    heat_transfer_coeff = _gap_conductance_constant / gap;
  }
  return _u_lambda(_qp) -
         heat_transfer_coeff * (_u_primal_slave(_qp) - (_has_master ? _u_primal_master(_qp) : 0));
}
