//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleTurbinePowerAux.h"

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
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addClassDescription("Computes turbine power for 1-phase flow for a simple on/off turbine");
  params.declareControllable("on");
  return params;
}

template <typename T>
SimpleTurbinePowerAuxTempl<T>::SimpleTurbinePowerAuxTempl(const InputParameters & parameters)
  : T(parameters), _on(this->template getParam<bool>("on"))
{
}

template <typename T>
Real
SimpleTurbinePowerAuxTempl<T>::computeValue()
{
  if (_on)
    return this->_value;
  else
    return 0.;
}

template class SimpleTurbinePowerAuxTempl<ConstantAux>;
template class SimpleTurbinePowerAuxTempl<ConstantScalarAux>;
