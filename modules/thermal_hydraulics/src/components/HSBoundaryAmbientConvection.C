//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryAmbientConvection.h"
#include "HeatConductionModel.h"
#include "HeatStructureCylindricalBase.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundaryAmbientConvection);

InputParameters
HSBoundaryAmbientConvection::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addRequiredParam<MooseFunctorName>(
      "htc_ambient", "Ambient Convective heat transfer coefficient functor [W/(m^2-K)]");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the boundary condition");
  params.addParam<bool>(
      "scale_heat_rate_pp",
      true,
      "If true, the 'scale' parameter is also applied to the heat rate post-processor.");

  params.addClassDescription("Applies a convective boundary condition to a heat structure");

  return params;
}

HSBoundaryAmbientConvection::HSBoundaryAmbientConvection(const InputParameters & params)
  : HSBoundary(params)
{
}

void
HSBoundaryAmbientConvection::addMooseObjects()
{
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name =
        is_cylindrical ? "ADConvectionHeatTransferRZBC" : "ADConvectionHeatTransferBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<MooseFunctorName>("T_ambient") = getParam<MooseFunctorName>("T_ambient");
    pars.set<MooseFunctorName>("htc_ambient") = getParam<MooseFunctorName>("htc_ambient");
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    }
    pars.set<MooseFunctorName>("scale") = getParam<MooseFunctorName>("scale");

    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }

  // Create integral PP for cylindrical heat structures
  if (is_cylindrical)
  {
    const std::string class_name = "HeatRateConvectionRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<std::vector<VariableName>>("T") = {HeatConductionModel::TEMPERATURE};
    pars.set<MooseFunctorName>("T_ambient") = getParam<MooseFunctorName>("T_ambient");
    pars.set<MooseFunctorName>("htc") = getParam<MooseFunctorName>("htc_ambient");
    pars.set<Point>("axis_point") = hs_cyl->getPosition();
    pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
    if (getParam<bool>("scale_heat_rate_pp"))
      pars.set<MooseFunctorName>("scale") = getParam<MooseFunctorName>("scale");
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, genSafeName(name(), "integral"), pars);
  }
}
