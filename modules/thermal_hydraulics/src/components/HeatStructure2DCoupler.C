//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCoupler.h"
#include "HeatStructureCylindricalBase.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DCoupler);

InputParameters
HeatStructure2DCoupler::validParams()
{
  InputParameters params = HeatStructure2DCouplerBase::validParams();

  params.addRequiredParam<FunctionName>("heat_transfer_coefficient",
                                        "Heat transfer coefficient function [W/(m^2-K)]");

  params.addClassDescription(
      "Couples boundaries of two 2D heat structures via a heat transfer coefficient");

  return params;
}

HeatStructure2DCoupler::HeatStructure2DCoupler(const InputParameters & parameters)
  : HeatStructure2DCouplerBase(parameters)
{
}

void
HeatStructure2DCoupler::check() const
{
  HeatStructure2DCouplerBase::check();

  if (!_mesh_alignment.meshesAreAligned())
    logError("The primary and secondary boundaries must be aligned.");
}

void
HeatStructure2DCoupler::addMooseObjects()
{
  HeatStructure2DCouplerBase::addMooseObjects();

  for (unsigned int i = 0; i < 2; i++)
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_names[i]);

    const std::string class_name =
        _is_cylindrical[i] ? "HeatStructure2DCouplerRZBC" : "HeatStructure2DCouplerBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::string>("coupled_variable") = HeatConductionModel::TEMPERATURE;
    params.set<std::vector<BoundaryName>>("boundary") = {_hs_boundaries[i]};
    params.set<MeshAlignment *>("_mesh_alignment") = &_mesh_alignment;
    params.set<FunctionName>("heat_transfer_coefficient") =
        getParam<FunctionName>("heat_transfer_coefficient");
    params.set<Real>("coupling_area_fraction") = _coupling_area_fractions[i];
    if (_is_cylindrical[i])
    {
      const HeatStructureCylindricalBase & hs_cylindrical =
          getComponentByName<HeatStructureCylindricalBase>(_hs_names[i]);

      params.set<Point>("axis_point") = hs.getPosition();
      params.set<RealVectorValue>("axis_dir") = hs.getDirection();
      params.set<Real>("offset") =
          hs_cylindrical.getInnerRadius() - hs_cylindrical.getAxialOffset();
    }
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), class_name, i), params);
  }
}
