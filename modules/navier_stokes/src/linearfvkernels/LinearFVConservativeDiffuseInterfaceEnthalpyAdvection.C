//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConservativeDiffuseInterfaceEnthalpyAdvection.h"
#include "LinearFVConservativeDiffuseInterfaceEnthalpyBC.h"
#include "ConservativeDiffuseInterfaceMixtureEnthalpy.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", LinearFVConservativeDiffuseInterfaceEnthalpyAdvection);

using Phase = ConservativeDiffuseInterfaceMixtureEnthalpy::Phase;

InputParameters
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Phase-wise upwind advection of the conserved mixture enthalpy "
                             "density of a diffuse-interface flow.");
  params.addRequiredParam<UserObjectName>(
      "mixture_enthalpy", "The ConservativeDiffuseInterfaceMixtureEnthalpy relating rho*h and T.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_flux",
      "alpha_phi_limited",
      "The area-integrated limited volume-fraction face flux of the volume-fraction corrector.");
  params.addParam<MooseFunctorName>(
      "mass_flux",
      "rho_phi",
      "The area-integrated mixture mass face flux of the volume-fraction corrector.");
  return params;
}

LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::
    LinearFVConservativeDiffuseInterfaceEnthalpyAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _mixture(getUserObject<ConservativeDiffuseInterfaceMixtureEnthalpy>("mixture_enthalpy")),
    _alpha_flux(getFunctor<Real>("volume_fraction_flux")),
    _mass_flux(getFunctor<Real>("mass_flux")),
    _phase_mass_flux({0.0, 0.0}),
    _elem_coeff(0.0),
    _neighbor_coeff(0.0),
    _flux_constant(0.0)
{
}

void
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::initialSetup()
{
  for (const auto & [_, bc] : _var.getBoundaryConditionMap())
    if (!dynamic_cast<const LinearFVConservativeDiffuseInterfaceEnthalpyBC *>(bc))
      mooseError(bc->type(), " is not a compatible boundary condition with ", type(), "!");
}

void
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::addPhaseFlux(const unsigned int phase,
                                                                    const Real mass_flux,
                                                                    const ElemInfo & upwind,
                                                                    const bool upwind_is_elem)
{
  const auto lin = _mixture.cellLinearization(upwind);
  const auto [h, cp] = _mixture.phaseEnthalpy(Phase(phase), lin.temperature);

  // m h(T) with T = T_lag + (rho*h - (rho*h)^*) / C
  (upwind_is_elem ? _elem_coeff : _neighbor_coeff) += mass_flux * cp / lin.heat_capacity;
  _flux_constant += mass_flux * (h - cp * lin.enthalpy_density / lin.heat_capacity);
}

void
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  _elem_coeff = 0.0;
  _neighbor_coeff = 0.0;
  _flux_constant = 0.0;

  // The two phase mass fluxes sum to the mixture mass flux the momentum equation advects with,
  // so the energy and momentum equations see the same mass crossing every face. The liquid flux
  // comes from the limited volume-fraction flux and the gas flux is what remains.
  const auto face = makeCDFace(*face_info);
  const auto state = determineState();
  const Real liquid_mass_flux =
      _mixture.phaseDensity(Phase::LIQUID, face) * _alpha_flux(face, state);
  _phase_mass_flux = {liquid_mass_flux, _mass_flux(face, state) - liquid_mass_flux};

  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return;

  for (const auto phase : index_range(_phase_mass_flux))
  {
    const Real mass_flux = _phase_mass_flux[phase];
    const bool upwind_is_elem = mass_flux >= 0.0;
    addPhaseFlux(phase,
                 mass_flux,
                 upwind_is_elem ? *face_info->elemInfo() : *face_info->neighborInfo(),
                 upwind_is_elem);
  }
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  computeBoundaryRHSContribution(bc);
  return _elem_coeff;
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyAdvection::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & bc)
{
  if (!_cached_rhs_contribution)
  {
    const auto & enthalpy_bc =
        static_cast<const LinearFVConservativeDiffuseInterfaceEnthalpyBC &>(bc);
    const auto & cell = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);
    // We support internal boundaries too, so orient the fluxes out of the cell
    const Real outward = NS::linearFVBoundaryNormalMultiplier(_current_face_type);

    for (const auto phase : index_range(_phase_mass_flux))
    {
      const Real mass_flux = outward * _phase_mass_flux[phase];
      // Entering fluid carries the prescribed temperature when there is one, else the cell's
      if (mass_flux < 0.0 && enthalpy_bc.hasTemperature())
        _flux_constant +=
            mass_flux *
            _mixture.phaseEnthalpy(Phase(phase), enthalpy_bc.boundaryTemperature()).first;
      else
        addPhaseFlux(phase, mass_flux, cell, /*upwind_is_elem=*/true);
    }
    _cached_rhs_contribution = true;
  }

  return -_flux_constant;
}
