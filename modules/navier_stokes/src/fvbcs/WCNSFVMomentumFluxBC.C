//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMomentumFluxBC.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMomentumFluxBC);

InputParameters
WCNSFVMomentumFluxBC::validParams()
{
  InputParameters params = WCNSFVFluxBCBase::validParams();

  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Flux boundary conditions for momentum advection.");

  return params;
}

WCNSFVMomentumFluxBC::WCNSFVMomentumFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params), INSFVMomentumResidualObject(*this)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to WCNSFVMomentumFluxBC must be of type INSFVVelocityVariable");

  // Need enough information to compute the mass flux
  if (_mdot_pp && !_area_pp)
    mooseError("The inlet area should be provided along with the mass flow rate");
  if (!_mdot_pp && !_velocity_pp)
    mooseError("Velocity should be provided if the mass flow rate is not");
}

ADReal
WCNSFVMomentumFluxBC::computeQpResidual()
{
  const auto state = determineState();

  if (!isInflow())
  {
    const auto fa = singleSidedFaceArg();
    const auto vel_vec = varVelocity(state);
    return vel_vec * _normal * _rho(fa, state) * vel_vec(_index);
  }

  const Point incoming_vector =
      !_direction_specified_by_user ? Point(-_face_info->normal()) : _direction;
  ADReal a = 1;
  if (_velocity_pp)
    a = 1.0 / std::abs(incoming_vector * _normal);
  return -_scaling_factor * a * inflowMassFlux(state) * inflowSpeed(state) *
         incoming_vector(_index);
}
