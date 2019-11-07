#include "HSBoundaryRadiation.h"
#include "HeatConductionModel.h"
#include "HeatStructureCylindrical.h"

registerMooseObject("THMApp", HSBoundaryRadiation);

template <>
InputParameters
validParams<HSBoundaryRadiation>()
{
  InputParameters params = validParams<HSBoundary>();

  params.addRequiredParam<Real>("emissivity", "Emissivity of flow channel [-]");
  params.addParam<FunctionName>("view_factor", "1", "View factor function [-]");
  params.addRequiredParam<FunctionName>("T_ambient", "Temperature of environment [K]");

  params.addClassDescription("Radiative heat transfer boundary condition for heat structure");

  return params;
}

HSBoundaryRadiation::HSBoundaryRadiation(const InputParameters & params) : HSBoundary(params) {}

void
HSBoundaryRadiation::addMooseObjects()
{
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  const HeatStructureCylindrical * hs_cyl = dynamic_cast<const HeatStructureCylindrical *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "RadiativeHeatFluxRZBC" : "RadiativeHeatFluxBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<Real>("boundary_emissivity") = getParam<Real>("emissivity");
    pars.set<FunctionName>("Tinfinity") = getParam<FunctionName>("T_ambient");
    pars.set<FunctionName>("view_factor") = getParam<FunctionName>("view_factor");
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs.getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius();
    }

    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
