//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  params.addRequiredParam<MooseFunctorName>("emissivity", "Emissivity functor [-]");
  params.addParam<MooseFunctorName>("view_factor", 1.0, "View factor functor [-]");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the boundary condition");
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
    pars.set<MooseFunctorName>("emissivity") = getParam<MooseFunctorName>("emissivity");
    pars.set<MooseFunctorName>("T_ambient") = getParam<MooseFunctorName>("T_ambient");
    pars.set<MooseFunctorName>("view_factor") = getParam<MooseFunctorName>("view_factor");
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    }
    pars.set<MooseFunctorName>("scale") = getParam<MooseFunctorName>("scale");
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
    pars.set<MooseFunctorName>("T_ambient") = getParam<MooseFunctorName>("T_ambient");
    pars.set<MooseFunctorName>("emissivity") = getParam<MooseFunctorName>("emissivity");
    pars.set<MooseFunctorName>("view_factor") = getParam<MooseFunctorName>("view_factor");
    pars.set<Point>("axis_point") = hs_cyl->getPosition();
    pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    if (getParam<bool>("scale_heat_rate_pp"))
      pars.set<MooseFunctorName>("scale") = getParam<MooseFunctorName>("scale");
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, genSafeName(name(), "integral"), pars);
  }
}
