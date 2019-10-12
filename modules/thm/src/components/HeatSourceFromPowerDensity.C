#include "HeatSourceFromPowerDensity.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindrical.h"
#include "HeatStructurePlate.h"

registerMooseObject("THMApp", HeatSourceFromPowerDensity);

template <>
InputParameters
validParams<HeatSourceFromPowerDensity>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "hs", "The name of the heat structure component to put the heat source onto");
  params.addRequiredParam<std::vector<std::string>>(
      "regions", "The names of the heat structure regions where heat generation is to be applied");
  params.addRequiredParam<VariableName>("power_density", "The name of the power density variable");
  params.addClassDescription("Heat source from power density");
  return params;
}

HeatSourceFromPowerDensity::HeatSourceFromPowerDensity(const InputParameters & parameters)
  : Component(parameters),
    _region_names(getParam<std::vector<std::string>>("regions")),
    _power_density_name(getParam<VariableName>("power_density"))
{
  checkSizeGreaterThan<std::string>("regions", 0);
}

void
HeatSourceFromPowerDensity::check() const
{
  Component::check();

  checkComponentOfTypeExists<HeatStructureBase>("hs");

  if (hasComponent<HeatStructureBase>("hs"))
  {
    if (!hasComponent<HeatStructurePlate>("hs") && !hasComponent<HeatStructureCylindrical>("hs"))
      logError(
          "Heat structure must be of type 'HeatStructurePlate' or 'HeatStructureCylindrical'.");

    const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");

    for (auto && region : _region_names)
      if (!hs.hasBlock(region))
        logError("Region '",
                 region,
                 "' does not exist in heat structure '",
                 getParam<std::string>("hs"),
                 "'.");
  }
}

void
HeatSourceFromPowerDensity::addMooseObjects()
{
  /// The heat structure component we work with
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  std::vector<SubdomainName> subdomain_names;
  for (auto && region : _region_names)
  {
    const unsigned int idx = hs.getIndexFromName(region);
    subdomain_names.push_back(hs.getSubdomainNames()[idx]);
  }

  const HeatStructureCylindrical * hs_cyl = dynamic_cast<const HeatStructureCylindrical *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "CoupledForceRZ" : "CoupledForce";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = subdomain_names;
    pars.set<std::vector<VariableName>>("v") = std::vector<VariableName>(1, _power_density_name);
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs.getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius();
    }
    std::string mon = genName(name(), "heat_src");
    _sim.addKernel(class_name, mon, pars);
  }
}
