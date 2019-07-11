#include "HSBoundaryHeatFlux.h"
#include "HeatConductionModel.h"
#include "HeatStructureCylindrical.h"

registerMooseObject("THMApp", HSBoundaryHeatFlux);

template <>
InputParameters
validParams<HSBoundaryHeatFlux>()
{
  InputParameters params = validParams<HSBoundary>();

  params.addRequiredParam<FunctionName>("q_function", "Heat flux function name");

  params.addClassDescription("Applies a specified heat flux to a heat structure boundary");

  return params;
}

HSBoundaryHeatFlux::HSBoundaryHeatFlux(const InputParameters & params)
  : HSBoundary(params),

    _q_fn_name(getParam<FunctionName>("q_function"))
{
}

void
HSBoundaryHeatFlux::addMooseObjects()
{
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  const HeatStructureCylindrical * hs_cyl = dynamic_cast<const HeatStructureCylindrical *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "HSHeatFluxRZBC" : "HSHeatFluxBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<FunctionName>("function") = _q_fn_name;
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs.getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius();
    }

    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }
}
