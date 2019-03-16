//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUO.h"

registerMooseObject("MooseTestApp", InterfaceUO);

template <>
InputParameters
validParams<InterfaceUO>()
{
  InputParameters params = validParams<InterfaceAverageUserObject>();
  params.addRequiredCoupledVar("var", "The variable name");
  params.addCoupledVar("var_neighbor", "The variable name");
  params.addClassDescription("Test Interface User Object computing the average value of two "
                             "variables across an interface");
  return params;
}

InterfaceUO::InterfaceUO(const InputParameters & parameters)
  : InterfaceAverageUserObject(parameters),
    _u(coupledValue("var")),
    _u_neighbor(parameters.isParamSetByUser("var_neighbor") ? coupledNeighborValue("var_neighbor")
                                                            : coupledNeighborValue("var")),
    _value(0.),
    _total_volume(0.)
{
}

InterfaceUO::~InterfaceUO() {}

void
InterfaceUO::initialize()
{
  _value = 0;
  _total_volume = 0;
}

void
InterfaceUO::execute()
{

  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
    _value += computeAverageType(_u[qp], _u_neighbor[qp]) * _JxW[qp];

  _total_volume += _current_side_volume;
}

void
InterfaceUO::finalize()
{
  gatherSum(_total_volume);
  gatherSum(_value);
  _value = _value / _total_volume;
}

void
InterfaceUO::threadJoin(const UserObject & uo)
{
  const InterfaceUO & u = dynamic_cast<const InterfaceUO &>(uo);
  _value += u._value;
  _total_volume += u._total_volume;
}

Real
InterfaceUO::getValue() const
{
  return _value;
}
