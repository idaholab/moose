//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowHeatAdvection.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowHeatAdvection);

InputParameters
FVPorousFlowHeatAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  RealVectorValue g(0, 0, -9.81);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector. Defaults to (0, 0, -9.81)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addClassDescription("Heat flux advected by the fluid");
  return params;
}

FVPorousFlowHeatAdvection::FVPorousFlowHeatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _density_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _viscosity(getADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _viscosity_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _enthalpy(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp")),
    _enthalpy_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp")),
    _relperm(getADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _relperm_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _permeability(getADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _permeability_neighbor(
        getNeighborADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _pressure(getADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _pressure_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _grad_p(getADMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
}

ADReal
FVPorousFlowHeatAdvection::computeQpResidual()
{
  ADReal flux = 0.0;
  ADRealGradient pressure_grad;
  ADRealTensorValue mobility;

  for (const auto p : make_range(_num_phases))
  {
    // If we are on a boundary face, use the gradient computed in _grad_p
    if (onBoundary(*_face_info))
    {
      const auto & gradp = -_grad_p[_qp][p];
      pressure_grad = gradp + _density[_qp][p] * _gravity;

      mobility = _enthalpy[_qp][p] * _relperm[_qp][p] * _permeability[_qp] * _density[_qp][p] /
                 _viscosity[_qp][p];
    }
    else
    {
      // If we are on an internal face, calculate the gradient explicitly
      const auto & p_elem = _pressure[_qp][p];
      const auto & p_neighbor = _pressure_neighbor[_qp][p];

      const auto gradp = (p_elem - p_neighbor) * _face_info->eCN() / _face_info->dCNMag();

      const auto mobility_element = _enthalpy[_qp][p] * _relperm[_qp][p] * _permeability[_qp] *
                                    _density[_qp][p] / _viscosity[_qp][p];

      const auto mobility_neighbor = _enthalpy_neighbor[_qp][p] * _relperm_neighbor[_qp][p] *
                                     _permeability_neighbor[_qp] * _density_neighbor[_qp][p] /
                                     _viscosity_neighbor[_qp][p];

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
