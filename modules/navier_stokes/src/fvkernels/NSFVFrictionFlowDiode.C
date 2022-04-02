//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVFrictionFlowDiode.h"
#include "NS.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

registerMooseObject("NavierStokesApp", NSFVFrictionFlowDiode);

InputParameters
NSFVFrictionFlowDiode::validParams()
{
  InputParameters params = INSFVElementalKernel::validParams();
  params.addClassDescription("Adds a linear friction term, -K vel_i * |d(i)|, i being the momentum"
                             "component, if the flow is opposite the direction d of the diode");
  params.addRequiredParam<Real>("resistance",
                                "Friction factor multiplying the superficial velocity if the flow "
                                "is in a half plane in the opposite direction of the normal");
  params.addRequiredParam<RealVectorValue>(
      "direction",
      "Normal direction of the diode. Flow is free in this half-plane, "
      "subject to friction in the other halfplane");

  return params;
}

NSFVFrictionFlowDiode::NSFVFrictionFlowDiode(const InputParameters & params)
  : INSFVElementalKernel(params),
    _direction(getParam<RealVectorValue>("direction")),
    _resistance(getParam<Real>("resistance"))
{
}

void
NSFVFrictionFlowDiode::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  const auto vel_comp = _var(elem_arg);

  if (vel_comp * _direction(_index) < 0)
  {
    const auto coefficient =
        _resistance * std::abs(_direction(_index)) * _assembly.elementVolume(&elem);

    _rc_uo.addToA(&elem, _index, coefficient);

    const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
    processResidual(coefficient * _u_functor(elem_arg), dof_number);
  }
}
