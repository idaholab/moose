#include "HSBoundarySpecifiedTemperature.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSBoundarySpecifiedTemperature);

template <>
InputParameters
validParams<HSBoundarySpecifiedTemperature>()
{
  InputParameters params = validParams<HSBoundary>();

  params.addRequiredParam<FunctionName>("T", "The function prescribing temperature");

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
    std::string class_name = "FunctionDirichletBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<FunctionName>("function") = _T_func;
    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
    makeFunctionControllableIfConstant(_T_func, "T");
  }
}
