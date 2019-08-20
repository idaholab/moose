#include "HeatConductionModel.h"
#include "Simulation.h"
#include "Factory.h"
#include "Component.h"
#include "THMApp.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindrical.h"

template <>
InputParameters
validParams<HeatConductionModel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<Simulation *>("_sim");
  params.addPrivateParam<HeatStructureBase *>("_hs");
  params.registerBase("THM:heat_conduction_model");
  return params;
}

registerMooseObject("THMApp", HeatConductionModel);

const std::string HeatConductionModel::DENSITY = "rho_solid";
const std::string HeatConductionModel::TEMPERATURE = "T_solid";
const std::string HeatConductionModel::THERMAL_CONDUCTIVITY = "k_solid";
const std::string HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE = "cp_solid";

FEType HeatConductionModel::_fe_type(FIRST, LAGRANGE);

HeatConductionModel::HeatConductionModel(const InputParameters & params)
  : MooseObject(params),
    _sim(*params.getCheckedPointerParam<Simulation *>("_sim")),
    _app(_sim.getApp()),
    _factory(_app.getFactory()),
    _hs(*params.getCheckedPointerParam<HeatStructureBase *>("_hs")),
    _comp_name(name())
{
}

void
HeatConductionModel::addVariables()
{
  const auto & subdomain_names = _hs.getSubdomainNames();
  const Real & scaling_factor = _sim.getParamTempl<Real>("scaling_factor_temperature");

  _sim.addVariable(true, TEMPERATURE, _fe_type, subdomain_names, scaling_factor);
}

void
HeatConductionModel::addInitialConditions()
{
  const auto & subdomain_names = _hs.getSubdomainNames();
  _sim.addFunctionIC(TEMPERATURE, _hs.getInitialT(), subdomain_names);
}

void
HeatConductionModel::addMaterials()
{
  const auto & blocks = _hs.getSubdomainNames();
  const auto & names = _hs.getParamTempl<std::vector<std::string>>("names");
  const auto & material_names = _hs.getParamTempl<std::vector<std::string>>("materials");

  for (std::size_t i = 0; i < names.size(); i++)
  {
    std::string class_name = "SolidMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = {blocks[i]};
    params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
    params.set<UserObjectName>("properties") = material_names[i];
    _sim.addMaterial(class_name, genName(_comp_name, names[i], "mat"), params);
  }
}

void
HeatConductionModel::addHeatEquationXYZ()
{
  const auto & blocks = _hs.getSubdomainNames();

  // add transient term
  {
    std::string class_name = "HeatConductionTimeDerivative";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
    pars.set<MaterialPropertyName>("density_name") = DENSITY;
    pars.set<bool>("use_displaced_mesh") = false;
    _sim.addKernel(class_name, genName(_comp_name, "td"), pars);
  }
  // add diffusion term
  {
    std::string class_name = "HeatConduction";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("diffusion_coefficient") = THERMAL_CONDUCTIVITY;
    pars.set<bool>("use_displaced_mesh") = false;
    _sim.addKernel(class_name, genName(_comp_name, "hc"), pars);
  }
}

void
HeatConductionModel::addHeatEquationRZ()
{
  HeatStructureCylindrical & hs_cyl = dynamic_cast<HeatStructureCylindrical &>(_hs);

  const auto & blocks = hs_cyl.getSubdomainNames();
  const auto & position = hs_cyl.getPosition();
  const auto & direction = hs_cyl.getDirection();
  const auto & inner_radius = hs_cyl.getInnerRadius();

  // add transient term
  {
    std::string class_name = "HeatConductionTimeDerivativeRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
    pars.set<MaterialPropertyName>("density_name") = DENSITY;
    pars.set<bool>("use_displaced_mesh") = false;
    pars.set<Point>("axis_point") = position;
    pars.set<RealVectorValue>("axis_dir") = direction;
    pars.set<Real>("offset") = inner_radius;
    _sim.addKernel(class_name, genName(_comp_name, "td"), pars);
  }
  // add diffusion term
  {
    std::string class_name = "HeatConductionRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = blocks;
    pars.set<MaterialPropertyName>("diffusion_coefficient") = THERMAL_CONDUCTIVITY;
    pars.set<bool>("use_displaced_mesh") = false;
    pars.set<Point>("axis_point") = position;
    pars.set<RealVectorValue>("axis_dir") = direction;
    pars.set<Real>("offset") = inner_radius;
    _sim.addKernel(class_name, genName(_comp_name, "hc"), pars);
  }
}
