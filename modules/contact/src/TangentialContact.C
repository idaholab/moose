//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialContact.h"

registerADMooseObject("ContactApp", TangentialContact);

defineADValidParams(TangentialContact,
                    ADMortarConstraint,

                    MooseEnum component("x=0 y=1 z=2");
                    params.addRequiredParam<MooseEnum>(
                        "component",
                        component,
                        "The force component constraint that this object is supplying"););

template <ComputeStage compute_stage>
TangentialContact<compute_stage>::TangentialContact(const InputParameters & parameters)
  : ADMortarConstraint<compute_stage>(parameters), _component(adGetParam<MooseEnum>("component"))
{
}

template <ComputeStage compute_stage>
ADReal
TangentialContact<compute_stage>::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Slave:
      // We have taken the convention the lagrange multiplier must have the same sign as the
      // relative slip velocity of the slave face. So positive lambda indicates that force is being
      // applied in the negative direction, so we want to decrease the momentum in the system, which
      // means we want an outflow of momentum, which means we want the residual to be positive in
      // that case. Negative lambda means force is being applied in the positive direction, so we
      // want to increase momentum in the system, which means we want an inflow of momentum, which
      // means we want the residual to be negative in that case. So the sign of this residual should
      // be the same as the sign of lambda
      return _test_slave[_i][_qp] * _lambda[_qp] *
             std::abs(_tangents[_qp][0](_component) / _tangents[_qp][0].norm());

    case Moose::MortarType::Master:
      // Equal and opposite reactions so we put a negative sign here
      return -_test_master[_i][_qp] * _lambda[_qp] *
             std::abs(_tangents[_qp][0](_component) / _tangents[_qp][0].norm());

    default:
      return 0;
  }
}
