//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundarySpecifiedTemperature.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundarySpecifiedTemperature);

InputParameters
HSBoundarySpecifiedTemperature::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addRequiredParam<FunctionName>("T", "Prescribed temperature [K]");

  params.declareControllable("T");

  params.addClassDescription("Applies Dirichlet boundary conditions on a heat structure");

  return params;
}

HSBoundarySpecifiedTemperature::HSBoundarySpecifiedTemperature(const InputParameters & params)
  : HSBoundary(params),

    _T_func(getParam<FunctionName>("T"))
{
}

void
HSBoundarySpecifiedTemperature::addMooseObjects()
{
  {
    std::string class_name = "ADFunctionDirichletBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<FunctionName>("function") = _T_func;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "bc"), pars);
    makeFunctionControllableIfConstant(_T_func, "T");
  }
}
