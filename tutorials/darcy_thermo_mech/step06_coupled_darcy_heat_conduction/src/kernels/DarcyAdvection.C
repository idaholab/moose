//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyAdvection.h"

registerMooseObject("DarcyThermoMechApp", DarcyAdvection);

InputParameters
DarcyAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("pressure", "The variable representing the pressure.");
  return params;
}

DarcyAdvection::DarcyAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    // Couple to the gradient of the pressure
    _pressure_grad(adCoupledGradient("pressure")),
    // Grab necessary material properties
    _permeability(getADMaterialProperty<Real>("permeability")),
    _porosity(getADMaterialProperty<Real>("porosity")),
    _viscosity(getADMaterialProperty<Real>("viscosity")),
    _density(getADMaterialProperty<Real>("density")),
    _specific_heat(getADMaterialProperty<Real>("specific_heat"))
{
}

ADReal
DarcyAdvection::precomputeQpResidual()
{
  // See also: E. Majchrzak and L. Turchan, "The Finite Difference
  // Method For Transient Convection Diffusion", Scientific Research
  // of the Institute of Mathematics and Computer Science, vol. 1,
  // no. 11, 2012, pp. 63-72.
  // http://srimcs.im.pcz.pl/2012_1/art_07.pdf

  // http://en.wikipedia.org/wiki/Superficial_velocity
  ADRealVectorValue superficial_velocity =
      _porosity[_qp] * -(_permeability[_qp] / _viscosity[_qp]) * _pressure_grad[_qp];

  return _density[_qp] * _specific_heat[_qp] * superficial_velocity * _grad_u[_qp];
}
