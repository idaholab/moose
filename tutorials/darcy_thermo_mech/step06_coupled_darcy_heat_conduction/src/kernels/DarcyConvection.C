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

  return params;
}

DarcyConvection::DarcyConvection(const InputParameters & parameters)
  : Kernel(parameters),

    // Couple to the gradient of the pressure
    _pressure_gradient(coupledGradient("darcy_pressure")),

    // Save off the coupled variable identifier for use in
    // computeQpOffDiagJacobian
    _pressure_var(coupled("darcy_pressure")),

    // Grab necessary material properties
    _permeability(getMaterialProperty<Real>("permeability")),
    _porosity(getMaterialProperty<Real>("porosity")),
    _viscosity(getMaterialProperty<Real>("viscosity")),
    _density(getMaterialProperty<Real>("density")),
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
  RealVectorValue superficial_velocity =
      _porosity[_qp] * -(_permeability[_qp] / _viscosity[_qp]) * _pressure_gradient[_qp];

  return _heat_capacity[_qp] * superficial_velocity * _grad_u[_qp] * _test[_i][_qp];
}

Real
DarcyConvection::computeQpJacobian()
{
  RealVectorValue superficial_velocity =
      _porosity[_qp] * -(_permeability[_qp] / _viscosity[_qp]) * _pressure_gradient[_qp];

  return _heat_capacity[_qp] * superficial_velocity * _grad_phi[_j][_qp] * _test[_i][_qp];
}

Real
DarcyConvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pressure_var)
  {
    RealVectorValue superficial_velocity =
        _porosity[_qp] * -(_permeability[_qp] / _viscosity[_qp]) * _grad_phi[_j][_qp];
    return _heat_capacity[_qp] * superficial_velocity * _grad_u[_qp] * _test[_i][_qp];
  }

  return 0.0;
}
