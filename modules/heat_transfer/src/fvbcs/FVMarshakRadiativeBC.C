//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMarshakRadiativeBC.h"
#include "MathUtils.h"
#include "HeatConductionNames.h"

registerMooseObject("HeatTransferApp", FVMarshakRadiativeBC);

InputParameters
FVMarshakRadiativeBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Marshak boundary condition for radiative heat flux.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiation temperature.");
  params.addRequiredParam<MooseFunctorName>("coeff_diffusion",
                                            "Radiative heat flux P1 diffusion coefficient.");
  params.addParam<Real>("boundary_emissivity", 1.0, "Emissivity of the boundary.");
  return params;
}

FVMarshakRadiativeBC::FVMarshakRadiativeBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _temperature_radiation(getFunctor<ADReal>("temperature_radiation")),
    _coeff_diffusion(getFunctor<ADReal>("coeff_diffusion")),
    _eps_boundary(getParam<Real>("boundary_emissivity"))
{
}

ADReal
FVMarshakRadiativeBC::computeQpResidual()
{
  const auto corrected_diff_coef =
      _eps_boundary / (2.0 * _coeff_diffusion(singleSidedFaceArg(_face_info), determineState()) *
                       (2.0 - _eps_boundary));
  const auto ground_flux =
      4.0 * HeatConduction::Constants::sigma *
      Utility::pow<4>(_temperature_radiation(singleSidedFaceArg(_face_info), determineState()));
  return -corrected_diff_coef *
         (_var(singleSidedFaceArg(_face_info), determineState()) - ground_flux);
}
