#include "HSBoundaryAmbientConvection.h"
#include "HeatConductionModel.h"
#include "HeatStructureCylindrical.h"

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
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  const bool is_cylindrical = dynamic_cast<const HeatStructureCylindrical *>(&hs);

  {
    const std::string class_name =
        is_cylindrical ? "ConvectionHeatTransferRZBC" : "ConvectionHeatTransferBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = {getHSBoundaryName(this)};
    pars.set<Real>("T_ambient") = _T_ambient;
    pars.set<Real>("htc_ambient") = _htc_ambient;
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs.getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
    }

    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
