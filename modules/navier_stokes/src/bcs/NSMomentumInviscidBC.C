//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMomentumInviscidBC.h"

InputParameters
NSMomentumInviscidBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addClassDescription("his class corresponds to the inviscid part of the 'natural' boundary "
                             "condition for the momentum equations.");
  params.addRequiredParam<unsigned>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  return params;
}

NSMomentumInviscidBC::NSMomentumInviscidBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _component(getParam<unsigned>("component")),
    // Object for computing deriviatives of pressure
    _pressure_derivs(*this)
{
}

Real
NSMomentumInviscidBC::pressureQpResidualHelper(Real pressure)
{
  // n . (Ip) . v

  // The pressure contribution: p * n(component) * phi_i
  Real press_term = pressure * _normals[_qp](_component) * _test[_i][_qp];

  // Return value, or print it first if debugging...
  return press_term;
}

Real
NSMomentumInviscidBC::pressureQpJacobianHelper(unsigned var_number)
{
  return _normals[_qp](_component) * _pressure_derivs.get_grad(var_number) * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
NSMomentumInviscidBC::convectiveQpResidualHelper(Real rhou_udotn)
{
  // n . (rho*uu) . v = rho*(u.n)*(u.v) = (rho*u)(u.n) . v

  // The "inviscid" contribution: (rho*u)(u.n) . v
  Real conv_term = rhou_udotn * _test[_i][_qp];

  // Return value, or print it first if debugging...
  return conv_term;
}

Real
NSMomentumInviscidBC::convectiveQpJacobianHelper(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Variable to store convective contribution to boundary integral.
  Real conv_term = 0.0;

  // Inviscid components
  switch (var_number)
  {
    case 0: // density
      // Note: the minus sign here is correct, it comes from differentiating wrt U_0
      // (rho) which is in the denominator.
      conv_term = -vel(_component) * (vel * _normals[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case 1:
    case 2:
    case 3: // momentums
      if (var_number - 1 == _component)
        // See Eqn. (68) from the notes for the inviscid boundary terms
        conv_term = ((vel * _normals[_qp]) + vel(_component) * _normals[_qp](_component)) *
                    _phi[_j][_qp] * _test[_i][_qp];
      else
        // off-diagonal
        conv_term =
            vel(_component) * _normals[_qp](var_number - 1) * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case 4: // energy
      // No derivative wrt energy
      conv_term = 0.0;
      break;

    default:
      mooseError("Shouldn't get here!");
      break;
  }

  // Return the result.  We could return it directly from the switch statement, but this is
  // convenient for printing...
  return conv_term;
}
