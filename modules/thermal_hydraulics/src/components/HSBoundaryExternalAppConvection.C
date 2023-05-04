//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryExternalAppConvection.h"
#include "HeatStructureCylindricalBase.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundaryExternalAppConvection);

InputParameters
HSBoundaryExternalAppConvection::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addParam<VariableName>("T_ext", "T_ext", "Temperature from external application");
  params.addParam<VariableName>(
      "htc_ext", "htc_ext", "Heat transfer coefficient from external application");
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

  params.addClassDescription("Heat structure boundary condition to perform convective heat "
                             "transfer with an external application");
  return params;
}

HSBoundaryExternalAppConvection::HSBoundaryExternalAppConvection(const InputParameters & params)
  : HSBoundary(params),

    _T_ext_var_name(getParam<VariableName>("T_ext")),
    _htc_ext_var_name(getParam<VariableName>("htc_ext"))
{
}

void
HSBoundaryExternalAppConvection::addVariables()
{
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const std::vector<SubdomainName> & subdomain_names =
      hs.getGeometricalComponent().getSubdomainNames();

  getTHMProblem().addSimVariable(
      false, _T_ext_var_name, HeatConductionModel::feType(), subdomain_names);
  getTHMProblem().addSimVariable(
      false, _htc_ext_var_name, HeatConductionModel::feType(), subdomain_names);
}

void
HSBoundaryExternalAppConvection::addMooseObjects()
{
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "ADExternalAppConvectionHeatTransferRZBC"
                                                  : "ADExternalAppConvectionHeatTransferBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<std::vector<VariableName>>("T_ext") = {_T_ext_var_name};
    pars.set<std::vector<VariableName>>("htc_ext") = {_htc_ext_var_name};
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
    const std::string class_name = "HeatRateExternalAppConvectionRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<std::vector<VariableName>>("T") = {HeatConductionModel::TEMPERATURE};
    pars.set<std::vector<VariableName>>("T_ext") = {_T_ext_var_name};
    pars.set<std::vector<VariableName>>("htc_ext") = {_htc_ext_var_name};
    pars.set<Point>("axis_point") = hs_cyl->getPosition();
    pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    if (getParam<bool>("scale_heat_rate_pp"))
      pars.set<FunctionName>("scale") = getParam<FunctionName>("scale");
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, genSafeName(name(), "integral"), pars);
  }
}
