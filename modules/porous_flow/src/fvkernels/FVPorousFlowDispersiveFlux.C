//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowDispersiveFlux.h"
#include "PorousFlowDictator.h"
#include "MooseUtils.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowDispersiveFlux);

InputParameters
FVPorousFlowDispersiveFlux::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  RealVectorValue g(0, 0, -9.81);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector. Defaults to (0, 0, -9.81)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addParam<unsigned int>("fluid_component", 0, "The fluid component");
  params.addRequiredParam<std::vector<Real>>(
      "disp_long", "Vector of longitudinal dispersion coefficients for each phase");
  params.addRequiredParam<std::vector<Real>>(
      "disp_trans", "Vector of transverse dispersion coefficients for each phase");
  params.addClassDescription(
      "Dispersive and diffusive flux of the component given by fluid_component in all phases");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addClassDescription("Advective Darcy flux");
  return params;
}

FVPorousFlowDispersiveFlux::FVPorousFlowDispersiveFlux(const InputParameters & params)
  : FVFluxKernel(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _viscosity(getADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _viscosity_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _relperm(getADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _relperm_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _mass_fractions(
        getADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _mass_fractions_neighbor(
        getNeighborADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _grad_mass_frac(getADMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "PorousFlow_grad_mass_frac_qp")),
    _permeability(getADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _permeability_neighbor(
        getNeighborADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _pressure(getADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _pressure_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _grad_p(getADMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _porosity(getADMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_neighbor(getNeighborADMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _tortuosity(getADMaterialProperty<std::vector<Real>>("PorousFlow_tortuosity_qp")),
    _tortuosity_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_tortuosity_qp")),
    _diffusion_coeff(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_diffusion_coeff_qp")),
    _diffusion_coeff_neighbor(getNeighborMaterialProperty<std::vector<std::vector<Real>>>(
        "PorousFlow_diffusion_coeff_qp")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _identity_tensor(ADRankTwoTensor::initIdentity),
    _disp_long(getParam<std::vector<Real>>("disp_long")),
    _disp_trans(getParam<std::vector<Real>>("disp_trans"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");

  // Check that sufficient values of the dispersion coefficients have been entered
  if (_disp_long.size() != _num_phases)
    paramError(
        "disp_long",
        "The number of longitudinal dispersion coefficients is not equal to the number of phases");

  if (_disp_trans.size() != _num_phases)
    paramError("disp_trans",
               "The number of transverse dispersion coefficients disp_trans is not equal to the "
               "number of phases");
}

ADReal
FVPorousFlowDispersiveFlux::computeQpResidual()
{
  ADReal flux = 0.0;
  ADRankTwoTensor dispersion;
  ADReal diffusion;

  for (unsigned int p = 0; p < _num_phases; ++p)
  {
    // Diffusive component
    ADRealGradient gradX;

    const auto state = determineState();
    const auto dudn = gradUDotNormal(state);

    // If we are on a boundary face, use the reconstructed gradient computed in _grad_mass_frac
    if (onBoundary(*_face_info))
    {
      gradX = -1 * _grad_mass_frac[_qp][p][_fluid_component];
      diffusion = _porosity[_qp] * _tortuosity[_qp][p] * _density[_qp][p] *
                  _diffusion_coeff[_qp][p][_fluid_component];
    }
    else
    {
      // If we are on an internal face, calculate the gradient explicitly
      const auto & X_elem = _mass_fractions[_qp][p][_fluid_component];
      const auto & X_neighbor = _mass_fractions_neighbor[_qp][p][_fluid_component];

      gradX = (X_elem - X_neighbor) * _face_info->eCN() / _face_info->dCNMag();

      const auto coeff =
          _porosity[_qp] * _tortuosity[_qp][p] * _diffusion_coeff[_qp][p][_fluid_component];

      const auto coeff_neighbor = _porosity_neighbor[_qp] * _tortuosity_neighbor[_qp][p] *
                                  _diffusion_coeff_neighbor[_qp][p][_fluid_component];

      interpolate(
          Moose::FV::InterpMethod::Average, diffusion, coeff, coeff_neighbor, *_face_info, true);
    }

    // Dispersive component. Calculate Darcy velocity
    ADRealGradient gradp;
    ADRealTensorValue mobility;

    // If we are on a boundary face, use the reconstructed gradient computed in _grad_p
    if (onBoundary(*_face_info))
    {
      gradp = -_grad_p[_qp][p] + _density[_qp][p] * _gravity;
      mobility = _relperm[_qp][p] * _permeability[_qp] * _density[_qp][p] / _viscosity[_qp][p];
    }
    else
    {
      // If we are on an internal face, calculate the gradient explicitly
      const auto & p_elem = _pressure[_qp][p];
      const auto & p_neighbor = _pressure_neighbor[_qp][p];

      gradp = (p_elem - p_neighbor) * _face_info->eCN() / _face_info->dCNMag() +
              _density[_qp][p] * _gravity;

      const auto mobility_element = _relperm[_qp][p] * _permeability[_qp] / _viscosity[_qp][p];

      const auto mobility_neighbor =
          _relperm_neighbor[_qp][p] * _permeability_neighbor[_qp] / _viscosity_neighbor[_qp][p];

      interpolate(Moose::FV::InterpMethod::Upwind,
                  mobility,
                  mobility_element,
                  mobility_neighbor,
                  gradp,
                  *_face_info,
                  true);
    }

    const auto velocity = mobility * gradp;
    const auto velocity_abs = MetaPhysicL::raw_value(velocity).norm();

    if (!MooseUtils::isZero(velocity_abs))
    {
      const auto v2 = ADRankTwoTensor::selfOuterProduct(velocity);

      // Add longitudinal dispersion to diffusive component
      diffusion += _disp_trans[p] * velocity_abs;
      dispersion = (_disp_long[p] - _disp_trans[p]) * v2 / velocity_abs;
    }

    flux += (diffusion * _identity_tensor + dispersion) * _density[_qp][p] * gradX * _normal;
  }

  return flux;
}
