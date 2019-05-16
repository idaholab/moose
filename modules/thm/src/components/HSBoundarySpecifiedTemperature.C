#include "HSBoundarySpecifiedTemperature.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSBoundarySpecifiedTemperature);

template <>
InputParameters
validParams<HSBoundarySpecifiedTemperature>()
{
  InputParameters params = validParams<BoundaryBase>();
  params += validParams<HSBoundaryInterface>();

  params.addRequiredParam<Real>("T", "The value of temperature");

  return params;
}

HSBoundarySpecifiedTemperature::HSBoundarySpecifiedTemperature(const InputParameters & params)
  : BoundaryBase(params),
    HSBoundaryInterface(this),

    _temperature(getParam<Real>("T"))
{
}

void
HSBoundarySpecifiedTemperature::check() const
{
  BoundaryBase::check();
  HSBoundaryInterface::check(this);
}

void
HSBoundarySpecifiedTemperature::addMooseObjects()
{
  {
    std::string class_name = "DirichletBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = {getHSBoundaryName(this)};
    pars.set<Real>("value") = _temperature;
    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
