#include "HeatGeneration.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindrical.h"
#include "HeatStructurePlate.h"
#include "ReactorPower.h"

registerMooseObject("THMApp", HeatGeneration);

template <>
InputParameters
validParams<HeatGeneration>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "hs", "The name of the heat structure component to put the heat source onto");
  params.addRequiredParam<std::vector<std::string>>(
      "regions", "The names of the heat structure regions where heat generation is to be applied");
  params.addParam<std::string>("power", "The component name that provides reactor power");
  params.addParam<Real>(
      "power_fraction", 1., "The fraction of reactor power that goes into the heat structure");
  params.addParam<FunctionName>("power_shape_function", "axial power shape of the fuel");
  params.addParam<std::string>("power_density", "The name of the power density variable");

  return params;
}

HeatGeneration::HeatGeneration(const InputParameters & parameters)
  : Component(parameters),
    _region_names(getParam<std::vector<std::string>>("regions")),
    _power_fraction(getParam<Real>("power_fraction")),
    _has_psf(isParamValid("power_shape_function")),
    _power_shape_func(_has_psf ? getParam<FunctionName>("power_shape_function") : ""),
    _has_power_density(isParamValid("power_density")),
    _power_density_name(_has_power_density ? getParam<std::string>("power_density") : "")
{
  checkSizeGreaterThan<std::string>("regions", 0);
}

void
HeatGeneration::init()
{
  Component::init();

  if (isParamValid("power"))
    if (hasComponent<ReactorPower>("power"))
    {
      const ReactorPower & rp = getComponent<ReactorPower>("power");
      _power_var_name = rp.getPowerVariableName();
    }
}

void
HeatGeneration::check() const
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

  checkMutuallyExclusiveParameters({"power", "power_density"});

  if (isParamValid("power"))
    checkComponentOfTypeExists<ReactorPower>("power");
}

void
HeatGeneration::addMooseObjects()
{
  /// The heat structure component we work with
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  Real volume = 0.;
  std::vector<SubdomainName> subdomain_names;
  for (auto && region : _region_names)
  {
    const unsigned int idx = hs.getIndexFromName(region);
    subdomain_names.push_back(hs.getSubdomainNames()[idx]);
    volume += hs.getVolumes()[idx];
  }

  const HeatStructureCylindrical * hs_cyl = dynamic_cast<const HeatStructureCylindrical *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  if (_has_power_density)
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
    }
    std::string mon = genName(name(), "heat_src");
    _sim.addKernel(class_name, mon, pars);
  }
  else
  {
    if (!_has_psf)
    {
      _power_shape_func = genName(name(), "power_shape_fn");
      std::string class_name = "ConstantFunction";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<Real>("value") = 1. / hs.getLength();
      _sim.addFunction(class_name, _power_shape_func, pars);
    }

    {
      const std::string class_name =
          is_cylindrical ? "OneDHeatForcingFunctionRZ" : "OneDHeatForcingFunction";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
      pars.set<std::vector<SubdomainName>>("block") = subdomain_names;
      pars.set<Real>("power_fraction") = _power_fraction;
      pars.set<Real>("volume") = volume;
      pars.set<FunctionName>("power_shape_function") = _power_shape_func;
      pars.set<std::vector<VariableName>>("total_power") =
          std::vector<VariableName>(1, _power_var_name);
      if (is_cylindrical)
      {
        pars.set<Point>("axis_point") = hs.getPosition();
        pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
      }
      std::string mon = genName(name(), "heat_src");
      _sim.addKernel(class_name, mon, pars);
      connectObject(pars, mon, "power_fraction");
    }
  }
}
