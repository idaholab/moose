//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaveHeightAuxKernel.h"

registerMooseObject("FsiApp", WaveHeightAuxKernel);

InputParameters
WaveHeightAuxKernel::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the wave heights given pressures.");
  params.addRequiredCoupledVar("pressure", "pressure variable");
  params.addRequiredParam<Real>("density", "fluid density");
  params.addRequiredParam<Real>("gravity", "acceleration due to gravity");
  return params;
}

WaveHeightAuxKernel::WaveHeightAuxKernel(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure(coupledValue("pressure")),
    _density(getParam<Real>("density")),
    _gravity(getParam<Real>("gravity"))
{
}

Real
WaveHeightAuxKernel::computeValue()
{
  // Vertical displacement = pressure/(density * gravity)
  return _pressure[_qp] / (_density * _gravity);
}
