//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryExternalAppTemperature.h"
#include "HeatConductionModel.h"
#include "HeatStructureInterface.h"
#include "GeometricalComponent.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundaryExternalAppTemperature);

InputParameters
HSBoundaryExternalAppTemperature::validParams()
{
  InputParameters params = HSBoundary::validParams();
  params.addParam<VariableName>(
      "T_ext",
      "T_ext",
      "Name of the variable that will store the values computed by the external application");
  params.addClassDescription("Heat structure boundary condition to set temperature values computed "
                             "by an external application");
  return params;
}

HSBoundaryExternalAppTemperature::HSBoundaryExternalAppTemperature(const InputParameters & params)
  : HSBoundary(params), _T_ext_var_name(getParam<VariableName>("T_ext"))
{
}

void
HSBoundaryExternalAppTemperature::addVariables()
{
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const std::vector<SubdomainName> & subdomain_names =
      hs.getGeometricalComponent().getSubdomainNames();

  getTHMProblem().addSimVariable(
      false, _T_ext_var_name, HeatConductionModel::feType(), subdomain_names);
}

void
HSBoundaryExternalAppTemperature::addMooseObjects()
{
  {
    std::string class_name = "ADMatchedValueBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<std::vector<VariableName>>("v") = {_T_ext_var_name};
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
