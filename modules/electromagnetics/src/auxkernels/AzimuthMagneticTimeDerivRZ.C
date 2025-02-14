//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AzimuthMagneticTimeDerivRZ.h"

registerMooseObject("ElectromagneticsApp", AzimuthMagneticTimeDerivRZ);

InputParameters
AzimuthMagneticTimeDerivRZ::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes the time derivative of the azimuthal component "
      "of the magnetic field assuming cylindrical electric field. The electric field can "
      "be supplied as a vector or scalar components.");
  params.addCoupledVar("Efield", "The electric field vector");
  params.addCoupledVar("Efield_X", "The x-component of the electric field");
  params.addCoupledVar("Efield_Y", "The y-component of the electric field");
  return params;
}

AzimuthMagneticTimeDerivRZ::AzimuthMagneticTimeDerivRZ(const InputParameters & parameters)
  : AuxKernel(parameters),
    _is_efield_vector(isCoupled("Efield")),
    _is_efield_scalar(isCoupled("Efield_X") && isCoupled("Efield_Y")),
    _efield_curl(_is_efield_vector ? coupledCurl("Efield") : _vector_curl_zero),
    _efield_x_grad(_is_efield_scalar ? coupledGradient("Efield_X") : _grad_zero),
    _efield_y_grad(_is_efield_scalar ? coupledGradient("Efield_Y") : _grad_zero)
{
  if (_is_efield_vector && _is_efield_scalar)
  {
    mooseError("Both a vector and scalar components of the electric field were provided! Please "
               "only choose one.");
  }

  if (!_is_efield_vector && !_is_efield_scalar)
  {
    mooseError("Neither a vector nor two scalar components of the electric field were provided! "
               "Please check the input parameters.");
  }
}

Real
AzimuthMagneticTimeDerivRZ::computeValue()
{
  if (_is_efield_vector)
  {
    /* NOTE: The curl for a axisymmetric cylindrical vector is equal and opposite to
     * the curl for 2D cartesian vector, such that:
     *    curl u_z = - curl u_theta
     * For Faraday's law of induction, this means we use the positive Cartesian curl,
     * in place of the negative axisymmetric cylindrical curl.
     */
    return _efield_curl[_qp](2);
  }
  else
  {
    return -(_efield_x_grad[_qp](1) - _efield_y_grad[_qp](0));
  }
}
