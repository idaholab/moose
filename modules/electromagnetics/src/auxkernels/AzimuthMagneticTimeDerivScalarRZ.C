//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AzimuthMagneticTimeDerivScalarRZ.h"

registerMooseObject("ElectromagneticsApp", AzimuthMagneticTimeDerivScalarRZ);

InputParameters
AzimuthMagneticTimeDerivScalarRZ::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes the time derivative of the azimuthal component "
      "of the magnetic field assuming cylindrical electric field. The electric field is "
      "is supplied as scalar components.");
  params.addRequiredCoupledVar("Efield_X", "The x-component of the E-field");
  params.addRequiredCoupledVar("Efield_Y", "The y-component of the E-field");
  return params;
}

AzimuthMagneticTimeDerivScalarRZ::AzimuthMagneticTimeDerivScalarRZ(const InputParameters & parameters)
  : AuxKernel(parameters),
    _efield_x_grad(coupledGradient("Efield_X")),
    _efield_y_grad(coupledGradient("Efield_Y"))
{
}

Real
AzimuthMagneticTimeDerivScalarRZ::computeValue()
{
  return -(_efield_x_grad[_qp](1) - _efield_y_grad[_qp](0));
}
