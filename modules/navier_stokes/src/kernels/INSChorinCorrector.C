//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSChorinCorrector.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSChorinCorrector);

InputParameters
INSChorinCorrector::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the 'Chorin' Corrector equation in "
                             "fully-discrete (both time and space) form.");
  // Coupled variables
  params.addRequiredCoupledVar("u_star", "star x-velocity");
  params.addCoupledVar("v_star", "star y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w_star", "star z-velocity"); // only required in 3D
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  // Required parameters
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the Corrector equation");

  // Optional parameters
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");

  return params;
}

INSChorinCorrector::INSChorinCorrector(const InputParameters & parameters)
  : Kernel(parameters),

    // Current velocities
    _u_vel_star(coupledValue("u_star")),
    _v_vel_star(_mesh.dimension() >= 2 ? coupledValue("v_star") : _zero),
    _w_vel_star(_mesh.dimension() == 3 ? coupledValue("w_star") : _zero),

    // Pressure gradient
    _grad_p(coupledGradient(NS::pressure)),

    // Variable numberings
    _u_vel_star_var_number(coupled("u_star")),
    _v_vel_star_var_number(_mesh.dimension() >= 2 ? coupled("v_star") : libMesh::invalid_uint),
    _w_vel_star_var_number(_mesh.dimension() == 3 ? coupled("w_star") : libMesh::invalid_uint),
    _p_var_number(coupled(NS::pressure)),

    // Required parameters
    _component(getParam<unsigned>("component")),

    // Material properties
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSChorinCorrector::computeQpResidual()
{
  // Vector object for U_star
  RealVectorValue U_star(_u_vel_star[_qp], _v_vel_star[_qp], _w_vel_star[_qp]);

  // The symmetric part
  Real symmetric_part = (_u[_qp] - U_star(_component)) * _test[_i][_qp];

  // The pressure part, don't forget to multiply by dt!
  Real pressure_part = (_dt / _rho[_qp]) * _grad_p[_qp](_component) * _test[_i][_qp];

  return symmetric_part + pressure_part;
}

Real
INSChorinCorrector::computeQpJacobian()
{
  // The on-diagonal Jacobian contribution is just the mass matrix entry.
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
INSChorinCorrector::computeQpOffDiagJacobian(unsigned jvar)
{
  if (((jvar == _u_vel_star_var_number) && (_component == 0)) ||
      ((jvar == _v_vel_star_var_number) && (_component == 1)) ||
      ((jvar == _w_vel_star_var_number) && (_component == 2)))
  {
    // The symmetric term's Jacobian is only non-zero when the
    // component of 'u_star' being differentiated is the same as _component.
    return -_phi[_j][_qp] * _test[_i][_qp];
  }

  else if (jvar == _p_var_number)
    return (_dt / _rho[_qp]) * _grad_phi[_j][_qp](_component) * _test[_i][_qp];

  else
    return 0;
}
