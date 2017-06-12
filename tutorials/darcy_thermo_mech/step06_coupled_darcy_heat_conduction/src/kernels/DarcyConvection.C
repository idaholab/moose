/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DarcyConvection.h"

template <>
InputParameters
validParams<DarcyConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("darcy_pressure", "The variable representing the pressure.");
  params.addRequiredCoupledVar("velocity_x",
                               "The variable representing the velocity in the x-direction.");
  params.addCoupledVar(
      "velocity_y", 0, "The variable representing the velocity in the y-direction.");
  params.addCoupledVar(
      "velocity_z", 0, "The variable representing the velocity in the z-direction.");
  return params;
}

DarcyConvection::DarcyConvection(const InputParameters & parameters)
  : Kernel(parameters),

    // Couple to the gradient of the pressure
    _pressure_gradient(coupledGradient("darcy_pressure")),

    // Save off the coupled variable identifier for use in
    // computeQpOffDiagJacobian
    _pressure_var(coupled("darcy_pressure")),

    // Store the couple velocity components
    _velocity_x(coupledValue("velocity_x")),
    _velocity_y(coupledValue("velocity_y")),
    _velocity_z(coupledValue("velocity_z")),

    // Grab necessary material property
    _heat_capacity(getMaterialProperty<Real>("heat_capacity"))
{
}

Real
DarcyConvection::computeQpResidual()
{
  // See also: E. Majchrzak and L. Turchan, "The Finite Difference
  // Method For Transient Convection Diffusion", Scientific Research
  // of the Institute of Mathematics and Computer Science, vol. 1,
  // no. 11, 2012, pp. 63-72.
  // http://srimcs.im.pcz.pl/2012_1/art_07.pdf

  // http://en.wikipedia.org/wiki/Superficial_velocity
  RealVectorValue superficial_velocity(_velocity_x[_qp], _velocity_y[_qp], _velocity_z[_qp]);
  return _heat_capacity[_qp] * superficial_velocity * _grad_u[_qp] * _test[_i][_qp];
}

Real
DarcyConvection::computeQpJacobian()
{
  RealVectorValue superficial_velocity(_velocity_x[_qp], _velocity_y[_qp], _velocity_z[_qp]);
  return _heat_capacity[_qp] * superficial_velocity * _grad_phi[_j][_qp] * _test[_i][_qp];
}

Real
DarcyConvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pressure_var)
  {
    RealVectorValue superficial_velocity(_velocity_x[_qp], _velocity_y[_qp], _velocity_z[_qp]);
    return _heat_capacity[_qp] * superficial_velocity * _grad_u[_qp] * _test[_i][_qp];
  }
  return 0.0;
}
