//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleTurbinePowerAux.h"
#include "THMUtils.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", SimpleTurbinePowerFieldAux);
registerMooseObject("ThermalHydraulicsApp", SimpleTurbinePowerScalarAux);
registerMooseObjectRenamed("ThermalHydraulicsApp",
                           SimpleTurbinePowerAux,
                           "10/31/2024 00:00",
                           SimpleTurbinePowerScalarAux);

template <typename T>
InputParameters
SimpleTurbinePowerAuxTempl<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addRequiredParam<FunctionName>(
      "on", "Function determining if turbine is operating (0=off, 1=on)");
  params.addRequiredParam<FunctionName>("power", "Function specifying the turbine power [W]");
  params.addClassDescription("Computes turbine power for 1-phase flow for a simple on/off turbine");
  return params;
}

template <typename T>
SimpleTurbinePowerAuxTempl<T>::SimpleTurbinePowerAuxTempl(const InputParameters & parameters)
  : T(parameters), _on_fn(this->getFunction("on")), _power_fn(this->getFunction("power"))
{
}

template <typename T>
Real
SimpleTurbinePowerAuxTempl<T>::computeValue()
{
  if (THM::realToBool(_on_fn.value(this->_t, Point())))
    return _power_fn.value(this->_t, Point());
  else
    return 0.;
}

template class SimpleTurbinePowerAuxTempl<ConstantAux>;
template class SimpleTurbinePowerAuxTempl<ConstantScalarAux>;
