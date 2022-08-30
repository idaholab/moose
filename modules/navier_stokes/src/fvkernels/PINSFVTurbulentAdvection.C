//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVTurbulentAdvection.h"
#include "INSFVEnergyVariable.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVTurbulentAdvection);

InputParameters
PINSFVTurbulentAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Advects turbulence energies or dissipation, "
                             " e.g. rho*TKE. A user may still override what "
                             "quantity is advected, but the default is rho*turbulent_energy");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  return params;
}

PINSFVTurbulentAdvection::PINSFVTurbulentAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params), _rho(getFunctor<ADReal>(NS::density))
{
}

ADReal
PINSFVTurbulentAdvection::computeQpResidual()
{
  ADReal adv_quant_interface;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  // Velocity interpolation
  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);

  // Interpolation of advected quantity
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _rho(elem_face) * _var(elem_face),
                         _rho(neighbor_face) * _var(neighbor_face),
                         v,
                         *_face_info,
                         true);

  return _normal * v * adv_quant_interface;
}