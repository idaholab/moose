#include "HSBoundarySpecifiedTemperature.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSBoundarySpecifiedTemperature);

template <>
InputParameters
validParams<HSBoundarySpecifiedTemperature>()
{
  InputParameters params = validParams<HSBoundary>();

  params.addRequiredParam<Real>("T", "The value of temperature");

  return params;
}

HSBoundarySpecifiedTemperature::HSBoundarySpecifiedTemperature(const InputParameters & params)
  : HSBoundary(params),

    _temperature(getParam<Real>("T"))
{
}

void
HSBoundarySpecifiedTemperature::addMooseObjects()
{
  {
    std::string class_name = "DirichletBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<Real>("value") = _temperature;
    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
