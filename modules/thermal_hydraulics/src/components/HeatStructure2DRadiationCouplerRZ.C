//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DRadiationCouplerRZ.h"
#include "HeatStructureCylindricalBase.h"
#include "HeatConductionNames.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DRadiationCouplerRZ);

InputParameters
HeatStructure2DRadiationCouplerRZ::validParams()
{
  InputParameters params = HeatStructure2DCouplerBase::validParams();

  params.addRequiredParam<Real>("primary_emissivity", "Emissivity for the primary side");
  params.addRequiredParam<Real>("secondary_emissivity", "Emissivity for the primary side");
  params.addParam<Real>("stefan_boltzmann_constant",
                        HeatConduction::Constants::sigma,
                        "Stefan Boltzmann constant [W/(m^2-K^4)]. This constant is provided as a "
                        "parameter to allow different precisions.");

  params.addClassDescription(
      "Couples boundaries of two 2D cylindrical heat structures via radiation");

  return params;
}

HeatStructure2DRadiationCouplerRZ::HeatStructure2DRadiationCouplerRZ(
    const InputParameters & parameters)
  : HeatStructure2DCouplerBase(parameters),

    _emissivities({getParam<Real>("primary_emissivity"), getParam<Real>("secondary_emissivity")})
{
}

void
HeatStructure2DRadiationCouplerRZ::init()
{
  HeatStructure2DCouplerBase::init();

  if (hasComponentByName<HeatStructureBase>(_hs_names[0]) &&
      hasComponentByName<HeatStructureBase>(_hs_names[1]) && _hs_side_types.size() == 2)
  {
    const HeatStructureBase & primary_hs = getComponentByName<HeatStructureBase>(_hs_names[0]);
    const HeatStructureBase & secondary_hs = getComponentByName<HeatStructureBase>(_hs_names[1]);

    _perimeters.resize(2);
    _perimeters[0] = primary_hs.getUnitPerimeter(_hs_side_types[0]);
    _perimeters[1] = secondary_hs.getUnitPerimeter(_hs_side_types[1]);

    _view_factors.resize(2);
    if (_perimeters[0] > _perimeters[1])
    {
      _view_factors[0] = _perimeters[1] / _perimeters[0];
      _view_factors[1] = 1.0;
    }
    else
    {
      _view_factors[0] = 1.0;
      _view_factors[1] = _perimeters[0] / _perimeters[1];
    }
  }
}

void
HeatStructure2DRadiationCouplerRZ::check() const
{
  HeatStructure2DCouplerBase::check();

  if (!_is_cylindrical[0] || !_is_cylindrical[1])
    logError("The primary and secondary heat structures must be of a type inherited from "
             "'HeatStructureCylindricalBase'.");

  if (!_mesh_alignment.meshesAreAligned())
    logError("The primary and secondary boundaries must be aligned.");
}

void
HeatStructure2DRadiationCouplerRZ::addMooseObjects()
{
  HeatStructure2DCouplerBase::addMooseObjects();

  for (unsigned int i = 0; i < 2; i++)
  {
    const unsigned int j = i == 0 ? 1 : 0;

    const std::string class_name = "HeatStructure2DRadiationCouplerRZBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::string>("coupled_variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_hs_boundaries[i]};
    params.set<MeshAlignment2D2D *>("_mesh_alignment") = &_mesh_alignment;
    params.set<Real>("emissivity") = _emissivities[i];
    params.set<Real>("coupled_emissivity") = _emissivities[j];
    params.set<Real>("view_factor") = _view_factors[i];
    params.set<Real>("perimeter") = _perimeters[i];
    params.set<Real>("coupled_perimeter") = _perimeters[j];
    params.set<Real>("stefan_boltzmann_constant") = getParam<Real>("stefan_boltzmann_constant");
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), class_name, i), params);
  }
}
