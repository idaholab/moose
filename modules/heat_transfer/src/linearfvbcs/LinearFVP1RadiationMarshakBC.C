//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVP1RadiationMarshakBC.h"
#include "HeatConductionNames.h"

registerMooseObject("HeatTransferApp", LinearFVP1RadiationMarshakBC);

InputParameters
LinearFVP1RadiationMarshakBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams();
  params.addClassDescription("Marshak boundary condition for radiative heat flux.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiation temperature.");
  params.addRequiredParam<MooseFunctorName>("coeff_diffusion",
                                            "Radiative heat flux P1 diffusion coefficient.");
  params.addParam<MooseFunctorName>("boundary_emissivity", 1.0, "Emissivity of the boundary.");
  return params;
}

LinearFVP1RadiationMarshakBC::LinearFVP1RadiationMarshakBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorRobinBCBase(parameters),
    _temperature_radiation(getFunctor<Real>("temperature_radiation")),
    _coeff_diffusion(getFunctor<Real>("coeff_diffusion")),
    _eps_boundary(getFunctor<Real>("boundary_emissivity"))
{
}

Real
LinearFVP1RadiationMarshakBC::getAlpha(Moose::FaceArg face, Moose::StateArg state) const
{
  const auto alpha = -_coeff_diffusion(face, state);
  return alpha;
}

Real
LinearFVP1RadiationMarshakBC::getBeta(Moose::FaceArg face, Moose::StateArg state) const
{

  const auto beta = -_eps_boundary(face, state) / (2 * (2 - _eps_boundary(face, state)));
  return beta;
}

Real
LinearFVP1RadiationMarshakBC::getGamma(Moose::FaceArg face, Moose::StateArg state) const
{

  const auto gamma = -_eps_boundary(face, state) * 4 * HeatConduction::Constants::sigma *
                     Utility::pow<4>(_temperature_radiation(face, state)) /
                     ((2 * (2 - _eps_boundary(face, state))));

  return gamma;
}
