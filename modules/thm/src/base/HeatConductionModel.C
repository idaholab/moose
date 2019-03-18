#include "HeatConductionModel.h"
#include "Simulation.h"
#include "Factory.h"
#include "Component.h"
#include "MooseApp.h"
#include "HeatStructure.h"

const std::string HeatConductionModel::DENSITY = "rho_solid";
const std::string HeatConductionModel::TEMPERATURE = "T_solid";
const std::string HeatConductionModel::THERMAL_CONDUCTIVITY = "k_solid";
const std::string HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE = "cp_solid";

FEType HeatConductionModel::_fe_type(FIRST, LAGRANGE);

HeatConductionModel::HeatConductionModel(const std::string & name, const InputParameters & params)
  : _hc_app(*params.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _hc_factory(_hc_app.getFactory()),
    _hc_sim(*params.getCheckedPointerParam<Simulation *>("_sim")),
    _comp_name(name)
{
}

void
HeatConductionModel::addVariables(InputParameters & cpars)
{
  _hc_sim.addVariable(true,
                      TEMPERATURE,
                      _fe_type,
                      cpars.get<std::vector<unsigned int>>("block_ids"),
                      _hc_sim.getParam<Real>("scaling_factor_temperature"));
}

void
HeatConductionModel::addMaterials(InputParameters & cpars)
{
  const auto & block = cpars.get<SubdomainName>("block");
  std::string name_of_hs = cpars.get<std::string>("name_of_hs");

  {
    std::string class_name = "SolidMaterial";
    InputParameters params = _hc_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = {block};
    params.set<std::vector<VariableName>>("T") = std::vector<VariableName>(1, TEMPERATURE);

    params.set<UserObjectName>("properties") = cpars.get<UserObjectName>("properties");
    _hc_sim.addMaterial(class_name, Component::genName("mat_" + name_of_hs, block, "hs"), params);
  }
}

void
HeatConductionModel::addHeatEquation(InputParameters & cpars)
{
  const auto & blocks = cpars.get<std::vector<SubdomainName>>("block");
  const auto & position = cpars.get<Point>("position");
  const auto & direction = cpars.get<RealVectorValue>("direction");

  HeatStructure::EHeatStructureType type = cpars.get<HeatStructure::EHeatStructureType>("hs_type");
  switch (type)
  {
    case HeatStructure::PLATE:
      // add transient term
      {
        std::string class_name = "HeatConductionTimeDerivative";
        InputParameters pars = _hc_factory.getValidParams(class_name);
        pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
        pars.set<std::vector<SubdomainName>>("block") = blocks;
        pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
        pars.set<MaterialPropertyName>("density_name") = DENSITY;
        pars.set<bool>("use_displaced_mesh") = false;
        _hc_sim.addKernel(class_name, Component::genName(_comp_name, "td"), pars);
      }
      // add diffusion term
      {
        std::string class_name = "HeatConduction";
        InputParameters pars = _hc_factory.getValidParams(class_name);
        pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
        pars.set<std::vector<SubdomainName>>("block") = blocks;
        pars.set<MaterialPropertyName>("diffusion_coefficient") = THERMAL_CONDUCTIVITY;
        pars.set<bool>("use_displaced_mesh") = false;
        _hc_sim.addKernel(class_name, Component::genName(_comp_name, "hc"), pars);
      }
      break;

    case HeatStructure::CYLINDER:
      // add transient term
      {
        std::string class_name = "HeatConductionTimeDerivativeRZ";
        InputParameters pars = _hc_factory.getValidParams(class_name);
        pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
        pars.set<std::vector<SubdomainName>>("block") = blocks;
        pars.set<MaterialPropertyName>("specific_heat") = SPECIFIC_HEAT_CONSTANT_PRESSURE;
        pars.set<MaterialPropertyName>("density_name") = DENSITY;
        pars.set<bool>("use_displaced_mesh") = false;
        pars.set<Point>("axis_point") = position;
        pars.set<RealVectorValue>("axis_dir") = direction;
        _hc_sim.addKernel(class_name, Component::genName(_comp_name, "td"), pars);
      }
      // add diffusion term
      {
        std::string class_name = "HeatConductionRZ";
        InputParameters pars = _hc_factory.getValidParams(class_name);
        pars.set<NonlinearVariableName>("variable") = TEMPERATURE;
        pars.set<std::vector<SubdomainName>>("block") = blocks;
        pars.set<MaterialPropertyName>("diffusion_coefficient") = THERMAL_CONDUCTIVITY;
        pars.set<bool>("use_displaced_mesh") = false;
        pars.set<Point>("axis_point") = position;
        pars.set<RealVectorValue>("axis_dir") = direction;
        _hc_sim.addKernel(class_name, Component::genName(_comp_name, "hc"), pars);
      }
      break;
  }
}
