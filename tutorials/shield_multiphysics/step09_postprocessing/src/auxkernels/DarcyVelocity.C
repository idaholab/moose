//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyVelocity.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("DarcyThermoMechApp", DarcyVelocity);

InputParameters
DarcyVelocity::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();

  // Add a "coupling paramater" to get a variable from the input file.
  params.addRequiredCoupledVar("pressure", "The pressure field.");

  return params;
}

DarcyVelocity::DarcyVelocity(const InputParameters & parameters)
  : VectorAuxKernel(parameters),

    // Get the gradient of the variable
    _pressure_gradient(coupledGradient("pressure")),

    // Set reference to the permeability MaterialProperty.
    // Only AuxKernels operating on Elemental Auxiliary Variables can do this
    _permeability(getADMaterialProperty<Real>("permeability")),

    // Set reference to the viscosity MaterialProperty.
    // Only AuxKernels operating on Elemental Auxiliary Variables can do this
    _viscosity(getADMaterialProperty<Real>("viscosity"))
{
}

RealVectorValue
DarcyVelocity::computeValue()
{
  // Access the gradient of the pressure at this quadrature point, then pull out the "component" of
  // it requested (x, y or z). Note, that getting a particular component of a gradient is done using
  // the parenthesis operator.
  return -MetaPhysicL::raw_value(_permeability[_qp] / _viscosity[_qp]) * _pressure_gradient[_qp];
}
