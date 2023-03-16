//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowAdvectiveFluxBC.h"
#include "PorousFlowDictator.h"
#include "FVDirichletBC.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowAdvectiveFluxBC);

InputParameters
FVPorousFlowAdvectiveFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  RealVectorValue g(0, 0, -9.81);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector. Defaults to (0, 0, -9.81)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addParam<unsigned int>("phase", 0, "The fluid phase for this BC");
  params.addParam<unsigned int>("fluid_component", 0, "The fluid component for this BC");
  params.addClassDescription("Advective Darcy flux boundary condition");
  params.addRequiredParam<Real>("porepressure_value", "The porepressure value on the boundary");
  return params;
}

FVPorousFlowAdvectiveFluxBC::FVPorousFlowAdvectiveFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _phase(getParam<unsigned int>("phase")),
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
    _gravity(getParam<RealVectorValue>("gravity")),
    _pp_value(getParam<Real>("porepressure_value"))
{
  if (_phase >= _num_phases)
    paramError(
        "phase",
        "The Dictator proclaims that the maximum fluid phase index in this simulation is ",
        _num_phases - 1,
        " whereas you have used ",
        _phase,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");

  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0.");

  // Add a FVDirichletBC to boundary for cell gradient computation
  if (_tid == 0)
  {
    auto diri_params = FVDirichletBC::validParams();
    diri_params.applySpecificParameters(_pars, {"variable", "boundary"});
    diri_params.addPrivateParam("_moose_app", &_app);
    diri_params.set<Real>("value") = _pp_value;
    _fv_problem.addFVBC("FVDirichletBC", name() + "_diri", diri_params);
    _fv_problem.fvBCsIntegrityCheck(false);
  }
}

ADReal
FVPorousFlowAdvectiveFluxBC::computeQpResidual()
{
  const bool out_of_elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM);

  const auto p_interior = out_of_elem ? _pressure[_qp][_phase] : _pressure_neighbor[_qp][_phase];
  const auto delta_p = out_of_elem ? p_interior - _pp_value : _pp_value - p_interior;
  const auto gradp = delta_p * _face_info->eCN() / _face_info->dCNMag();

  const auto mobility =
      out_of_elem ? _mass_fractions[_qp][_phase][_fluid_component] * _relperm[_qp][_phase] *
                        _permeability[_qp] * _density[_qp][_phase] / _viscosity[_qp][_phase]
                  : _mass_fractions_neighbor[_qp][_phase][_fluid_component] *
                        _relperm_neighbor[_qp][_phase] * _permeability_neighbor[_qp] *
                        _density_neighbor[_qp][_phase] / _viscosity_neighbor[_qp][_phase];

  const auto pressure_grad = gradp + _density[_qp][_phase] * _gravity;

  return mobility * pressure_grad * _normal;
}
