//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryRadiation.h"
#include "HeatConductionModel.h"
#include "HeatStructureInterface.h"
#include "HeatStructureCylindricalBase.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundaryRadiation);

InputParameters
HSBoundaryRadiation::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addRequiredParam<Real>("emissivity", "Emissivity of flow channel [-]");
  params.addParam<FunctionName>("view_factor", "1", "View factor function [-]");
  params.addRequiredParam<FunctionName>("T_ambient", "Temperature of environment [K]");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");
  params.addParam<bool>(
      "scale_heat_rate_pp",
      true,
      "If true, the scaling function is applied to the heat rate post-processor.");

  params.addClassDescription("Radiative heat transfer boundary condition for heat structure");

  return params;
}

HSBoundaryRadiation::HSBoundaryRadiation(const InputParameters & params) : HSBoundary(params) {}

void
HSBoundaryRadiation::addMooseObjects()
{
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name =
        is_cylindrical ? "ADRadiativeHeatFluxRZBC" : "ADRadiativeHeatFluxBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<Real>("boundary_emissivity") = getParam<Real>("emissivity");
    pars.set<FunctionName>("Tinfinity") = getParam<FunctionName>("T_ambient");
    pars.set<FunctionName>("view_factor") = getParam<FunctionName>("view_factor");
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    }
    pars.set<FunctionName>("scale") = getParam<FunctionName>("scale");
    if (isParamValid("scale_pp"))
      pars.set<PostprocessorName>("scale_pp") = getParam<PostprocessorName>("scale_pp");

    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }

  // Create integral PP for cylindrical heat structures
  if (is_cylindrical)
  {
    const std::string class_name = "HeatRateRadiationRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<std::vector<VariableName>>("T") = {HeatConductionModel::TEMPERATURE};
    pars.set<FunctionName>("T_ambient") = getParam<FunctionName>("T_ambient");
    pars.set<Real>("emissivity") = getParam<Real>("emissivity");
    pars.set<FunctionName>("view_factor") = getParam<FunctionName>("view_factor");
    pars.set<Point>("axis_point") = hs_cyl->getPosition();
    pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    if (getParam<bool>("scale_heat_rate_pp"))
      pars.set<FunctionName>("scale") = getParam<FunctionName>("scale");
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, genSafeName(name(), "integral"), pars);
  }
}
