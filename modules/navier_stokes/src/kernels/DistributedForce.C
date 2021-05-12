//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedForce.h"

registerMooseObject("NavierStokesApp", DistributedForce);

InputParameters
DistributedForce::validParams()
{
  InputParameters params = Kernel::validParams();

  // The acceleration vector.
  params.addParam<RealVectorValue>(
      "acceleration",
      RealVectorValue(0, 0, 0),
      "The acceleration components for an applied distributed force in an element.");
  params.addRequiredParam<unsigned int>("component", "acceleration vector components");

  // The body force acts on the mass of the volume
  params.addRequiredCoupledVar("rho", "density"); // Density integrated over a volume yields mass
  params.addClassDescription("Implements a force term in the Navier Stokes momentum equation.");

  return params;
}

DistributedForce::DistributedForce(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _acceleration(getParam<RealVectorValue>("acceleration")(_component)),
    _rho_var_number(coupled("rho")),
    _rho(coupledValue("rho"))
{
}

Real
DistributedForce::computeQpResidual()
{
  // -rho * g * phi
  return -_rho[_qp] * _acceleration * _test[_i][_qp];
}

Real
DistributedForce::computeQpJacobian()
{
  return 0.;
}

Real
DistributedForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rho_var_number)
    return -_phi[_j][_qp] * _acceleration * _test[_i][_qp];

  return 0;
}
