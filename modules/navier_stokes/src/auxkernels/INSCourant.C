//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSCourant.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSCourant);

InputParameters
INSCourant::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Computes h_min / |u|.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  return params;
}

INSCourant::INSCourant(const InputParameters & parameters)
  : AuxKernel(parameters),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero)
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
INSCourant::computeValue()
{
  const RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real vel_mag = U.norm();

  // Don't divide by zero...
  vel_mag = std::max(vel_mag, std::numeric_limits<Real>::epsilon());

  return _current_elem->hmin() / vel_mag;
}
