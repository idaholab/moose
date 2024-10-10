//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AzimuthMagneticTimeDerivVector.h"

registerMooseObject("ElectromagneticsApp", AzimuthMagneticTimeDerivVector);

InputParameters
AzimuthMagneticTimeDerivVector::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes the time derivative of the azimuthal component "
      "of the magnetic field assuming cylindrical electric field. The electric field is "
      "is supplied as a vector.");
  params.addRequiredCoupledVar("Efield", "The electric field vector");
  return params;
}

AzimuthMagneticTimeDerivVector::AzimuthMagneticTimeDerivVector(const InputParameters & parameters)
  : AuxKernel(parameters), _efield_curl(coupledCurl("Efield"))
{
}

Real
AzimuthMagneticTimeDerivVector::computeValue()
{
  /* NOTE: The curl for a axisymmetric cylindrical vector is equal and opposite to
   * the curl for 2D cartesian vector, such that:
   *    curl u_z = - curl u_theta
   * For Faraday's law of induction, this means we use the positive cartesian curl,
   * in place of the negitive axisymmetric cylindrical curl.
   */
  return _efield_curl[_qp](2);
}
