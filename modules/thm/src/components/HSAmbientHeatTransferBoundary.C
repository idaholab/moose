#include "HSAmbientHeatTransferBoundary.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSAmbientHeatTransferBoundary);

template <>
InputParameters
validParams<HSAmbientHeatTransferBoundary>()
{
  InputParameters params = validParams<BoundaryBase>();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The name of the boundary where the boundary condition is imposed.");
  params.addRequiredParam<Real>("htc_ambient", "Convective heat transfer coefficient with ambient");
  params.addRequiredParam<Real>("T_ambient", "Ambient temperature");
  return params;
}

HSAmbientHeatTransferBoundary::HSAmbientHeatTransferBoundary(const InputParameters & params)
  : BoundaryBase(params),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _T_ambient(getParam<Real>("T_ambient")),
    _htc_ambient(getParam<Real>("htc_ambient"))
{
}

void
HSAmbientHeatTransferBoundary::addMooseObjects()
{
  {
    std::string class_name = "ConvectionHeatTransferBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<Real>("T_ambient") = _T_ambient;
    pars.set<Real>("htc_ambient") = _htc_ambient;
    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
