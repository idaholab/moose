#include "Lapidus.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "Component.h"
#include "InputParameterLogic.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", Lapidus);

template <>
InputParameters
validParams<Lapidus>()
{
  InputParameters params = validParams<StabilizationSettings>();
  params.addParam<Real>("cl", 1., "Coefficient for single phase.");
  params.addParam<Real>("cl_liquid", 1., "Coefficient for liquid phase.");
  params.addParam<Real>("cl_vapor", 1., "Coefficient for vapor phase.");
  return params;
}

Lapidus::Lapidus(const InputParameters & parameters)
  : StabilizationSettings(parameters),
    _cl(getParam<Real>("cl")),
    _cl_liquid(getParam<Real>("cl_liquid")),
    _cl_vapor(getParam<Real>("cl_vapor"))
{
}

void
Lapidus::addVariables(FlowModel &, unsigned int) const
{
}

void
Lapidus::initMooseObjects(FlowModel & fm)
{
  bool isTwoPhase = dynamic_cast<FlowModelTwoPhase *>(&fm);
  getOneOrTwoPhaseParameters<Real>(isTwoPhase, "cl", {"cl_liquid", "cl_vapor"}, *this);
}

void
Lapidus::addMooseObjects(FlowModel & fm, InputParameters & pars) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm) != NULL)
    setup1Phase(dynamic_cast<FlowModelSinglePhase &>(fm), pars);
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL)
    setup2Phase(dynamic_cast<FlowModelTwoPhase &>(fm), pars);
}

void
Lapidus::setup1Phase(FlowModelSinglePhase & /*fm*/, InputParameters & pars) const
{
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");

  std::vector<NonlinearVariableName> vars;
  std::vector<VariableName> U;
  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  {
    std::string class_name = "LapidusCoefMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<Real>("cl") = _cl;
    params.set<std::vector<VariableName>>("vel") =
        std::vector<VariableName>(1, FlowModelSinglePhase::VELOCITY);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef";
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    _m_sim.addMaterial(class_name, Component::genName(comp_name, "lapidus_material"), params);
  }

  vars.push_back(FlowModelSinglePhase::RHOA);
  vars.push_back(FlowModelSinglePhase::RHOUA);
  vars.push_back(FlowModelSinglePhase::RHOEA);

  U.push_back(FlowModelSinglePhase::DENSITY);
  U.push_back(FlowModelSinglePhase::MOMENTUM_DENSITY);
  U.push_back(FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);

  for (unsigned int i = 0; i < vars.size(); i++)
  {
    std::string class_name = "OneDArtificialDissipation";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("U") = std::vector<VariableName>(1, U[i]);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef";
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "lapidus"), params);
  }
}

void
Lapidus::setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const
{
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");

  std::vector<NonlinearVariableName> vars;
  std::vector<VariableName> U;
  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  {
    std::string class_name = "LapidusCoefMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<Real>("cl") = _cl_liquid;
    params.set<std::vector<VariableName>>("vel") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VELOCITY_LIQUID);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef_liquid";
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    _m_sim.addMaterial(
        class_name, Component::genName(comp_name, "lapidus_material_liquid"), params);
  }
  {
    std::string class_name = "LapidusCoefMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<Real>("cl") = _cl_vapor;
    params.set<std::vector<VariableName>>("vel") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VELOCITY_VAPOR);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef_vapor";
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    _m_sim.addMaterial(class_name, Component::genName(comp_name, "lapidus_material_vapor"), params);
  }

  vars.push_back(FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);

  U.push_back(FlowModelTwoPhase::DENSITY_LIQUID);
  U.push_back(FlowModelTwoPhase::MOMENTUM_DENSITY_LIQUID);
  U.push_back(FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_LIQUID);
  U.push_back(FlowModelTwoPhase::DENSITY_VAPOR);
  U.push_back(FlowModelTwoPhase::MOMENTUM_DENSITY_VAPOR);
  U.push_back(FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_VAPOR);

  // liquid
  for (unsigned int i = 0; i < 3; i++)
  {
    std::string class_name = "OneDArtificialDissipation";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("U") = std::vector<VariableName>(1, U[i]);
    params.set<std::vector<VariableName>>("alpha") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef_liquid";
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "lapidus"), params);
  }
  // vapor
  for (unsigned int i = 3; i < 6; i++)
  {
    std::string class_name = "OneDArtificialDissipation";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("U") = std::vector<VariableName>(1, U[i]);
    params.set<std::vector<VariableName>>("alpha") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
    params.set<MaterialPropertyName>("coef_name") = "lapidus_coef_vapor";
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "lapidus"), params);
  }

  if (fm.getPhaseInteraction())
  {
    // volume fraction
    {
      std::string class_name = "OneDArtificialDissipation";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
      params.set<std::vector<SubdomainName>>("block") = blocks;
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("U") =
          std::vector<VariableName>(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
      params.set<MaterialPropertyName>("coef_name") = "lapidus_coef_liquid";
      _m_sim.addKernel(class_name, Component::genName(comp_name, "alpha_A_liquid_lapidus"), params);
    }
  }
}
