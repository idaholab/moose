#include "HSSpecifiedTemperatureBoundary.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSSpecifiedTemperatureBoundary);

template <>
InputParameters
validParams<HSSpecifiedTemperatureBoundary>()
{
  InputParameters params = validParams<BoundaryBase>();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The name of the boundary where the boundary condition is imposed.");
  params.addRequiredParam<Real>("T", "The value of temperature");
  return params;
}

HSSpecifiedTemperatureBoundary::HSSpecifiedTemperatureBoundary(const InputParameters & params)
  : BoundaryBase(params),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _temperature(getParam<Real>("T"))
{
}

void
HSSpecifiedTemperatureBoundary::addMooseObjects()
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
