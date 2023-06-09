//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowAdvectiveFlux.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowAdvectiveFlux);

InputParameters
FVPorousFlowAdvectiveFlux::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  RealVectorValue g(0, 0, -9.81);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector. Defaults to (0, 0, -9.81)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addParam<unsigned int>("fluid_component", 0, "The fluid component for this kernel");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addClassDescription("Advective Darcy flux");
  return params;
}

FVPorousFlowAdvectiveFlux::FVPorousFlowAdvectiveFlux(const InputParameters & params)
  : FVFluxKernel(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _density_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
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
    _permeability(getADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _permeability_neighbor(
        getNeighborADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _pressure(getADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _pressure_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _grad_p(getADMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

ADReal
FVPorousFlowAdvectiveFlux::computeQpResidual()
{
  ADReal flux = 0.0;
  ADRealGradient pressure_grad;
  ADRealTensorValue mobility;

  for (unsigned int p = 0; p < _num_phases; ++p)
  {
    // If we are on a boundary face, use the reconstructed gradient computed in _grad_p
    if (onBoundary(*_face_info))
    {
      const auto & gradp = -_grad_p[_qp][p];
      pressure_grad = gradp + _density[_qp][p] * _gravity;

      mobility = _mass_fractions[_qp][p][_fluid_component] * _relperm[_qp][p] * _permeability[_qp] *
                 _density[_qp][p] / _viscosity[_qp][p];
    }
    else
    {
      // If we are on an internal face, calculate the gradient explicitly
      const auto & p_elem = _pressure[_qp][p];
      const auto & p_neighbor = _pressure_neighbor[_qp][p];

      const auto gradp = (p_elem - p_neighbor) * _face_info->eCN() / _face_info->dCNMag();

      const auto mobility_element = _mass_fractions[_qp][p][_fluid_component] * _relperm[_qp][p] *
                                    _permeability[_qp] * _density[_qp][p] / _viscosity[_qp][p];

      const auto mobility_neighbor = _mass_fractions_neighbor[_qp][p][_fluid_component] *
                                     _relperm_neighbor[_qp][p] * _permeability_neighbor[_qp] *
                                     _density_neighbor[_qp][p] / _viscosity_neighbor[_qp][p];

      pressure_grad = gradp + _density[_qp][p] * _gravity;

      interpolate(Moose::FV::InterpMethod::Upwind,
                  mobility,
                  mobility_element,
                  mobility_neighbor,
                  pressure_grad,
                  *_face_info,
                  true);
    }

    flux += mobility * pressure_grad * _normal;
  }

  return flux;
}
