#include "HSBoundaryAmbientConvection.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HSBoundaryAmbientConvection);

template <>
InputParameters
validParams<HSBoundaryAmbientConvection>()
{
  InputParameters params = validParams<BoundaryBase>();
  params += validParams<HSBoundaryInterface>();

  params.addRequiredParam<Real>("htc_ambient", "Convective heat transfer coefficient with ambient");
  params.addRequiredParam<Real>("T_ambient", "Ambient temperature");

  return params;
}

HSBoundaryAmbientConvection::HSBoundaryAmbientConvection(const InputParameters & params)
  : BoundaryBase(params),
    HSBoundaryInterface(this),

    _T_ambient(getParam<Real>("T_ambient")),
    _htc_ambient(getParam<Real>("htc_ambient"))
{
}

void
HSBoundaryAmbientConvection::check() const
{
  BoundaryBase::check();
  HSBoundaryInterface::check(this);
}

void
HSBoundaryAmbientConvection::addMooseObjects()
{
  {
    std::string class_name = "ConvectionHeatTransferBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = {getHSBoundaryName(this)};
    pars.set<Real>("T_ambient") = _T_ambient;
    pars.set<Real>("htc_ambient") = _htc_ambient;
    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
