//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSImposedVelocityDirectionBC.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

// Full specialization of the validParams function for this object
registerMooseObject("NavierStokesApp", NSImposedVelocityDirectionBC);

InputParameters
NSImposedVelocityDirectionBC::validParams()
{
  // Initialize the params object from the base class
  InputParameters params = NodalBC::validParams();

  params.addClassDescription("This class imposes a velocity direction component as a Dirichlet "
                             "condition on the appropriate momentum equation.");
  // Coupled variables
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // only required in 3D

  // Coupled parameters
  params.addRequiredParam<Real>("desired_unit_velocity_component", "");

  return params;
}

NSImposedVelocityDirectionBC::NSImposedVelocityDirectionBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _rho(coupledValue(NS::density)),
    _u_vel(coupledValue(NS::velocity_x)),
    _v_vel(_mesh.dimension() == 2 ? coupledValue(NS::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(NS::velocity_z) : _zero),
    _desired_unit_velocity_component(getParam<Real>("desired_unit_velocity_component"))
{
}

Real
NSImposedVelocityDirectionBC::computeQpResidual()
{
  // The velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Specify desired velocity component
  return _u[_qp] - _rho[_qp] * _desired_unit_velocity_component * vel.norm();
}
