//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSGravityForce.h"

registerMooseObject("NavierStokesApp", NSGravityForce);

InputParameters
NSGravityForce::validParams()
{
  InputParameters params = NSKernel::validParams();
  params.addClassDescription("This class computes the gravity force contribution.");
  // The strength of the acceleration in the _component direction.  Make this
  // value negative if you want force in the -_component direction.
  params.addRequiredParam<Real>("acceleration", "The body force vector component.");
  return params;
}

NSGravityForce::NSGravityForce(const InputParameters & parameters)
  : NSKernel(parameters), _acceleration(getParam<Real>("acceleration"))
{
}

Real
NSGravityForce::computeQpResidual()
{
  // -rho * g * phi
  return -_rho[_qp] * _acceleration * _test[_i][_qp];
}

Real
NSGravityForce::computeQpJacobian()
{
  return 0.0;
}

Real
NSGravityForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rho_var_number)
    return -_phi[_j][_qp] * _acceleration * _test[_i][_qp];

  return 0.0;
}
