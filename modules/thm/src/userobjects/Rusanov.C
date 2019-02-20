#include "Rusanov.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "Component.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("THMApp", Rusanov);

template <>
InputParameters
validParams<Rusanov>()
{
  InputParameters params = validParams<StabilizationSettings>();
  return params;
}

Rusanov::Rusanov(const InputParameters & parameters) : StabilizationSettings(parameters) {}

void
Rusanov::addVariables(FlowModel &, unsigned int) const
{
}

void
Rusanov::initMooseObjects(FlowModel &)
{
}

void
Rusanov::addMooseObjects(FlowModel & fm, InputParameters & pars) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm))
    setup1Phase(dynamic_cast<FlowModelSinglePhase &>(fm), pars);
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm))
    setup2Phase(dynamic_cast<FlowModelTwoPhase &>(fm), pars);
}

void
Rusanov::setup1Phase(FlowModelSinglePhase & /*fm*/, InputParameters & pars) const
{
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  std::vector<NonlinearVariableName> vars;
  vars.push_back(FlowModelSinglePhase::RHOA);
  vars.push_back(FlowModelSinglePhase::RHOUA);
  vars.push_back(FlowModelSinglePhase::RHOEA);

  for (unsigned int i = 0; i < vars.size(); i++)
  {
    std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);

    std::string class_name = "OneDRusanov";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("c") = FlowModelSinglePhase::SOUND_SPEED;
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "rusanov"), params);
  }
}

void
Rusanov::setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const
{
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  std::vector<NonlinearVariableName> vars;
  vars.push_back(FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  vars.push_back(FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);

  // liquid
  for (unsigned int i = 0; i < 3; i++)
  {
    std::vector<VariableName> cv_vel(1, FlowModelTwoPhase::VELOCITY_LIQUID);

    std::string class_name = "OneDRusanov";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_LIQUID;
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "rusanov"), params);
  }
  // vapor
  for (unsigned int i = 3; i < 6; i++)
  {
    std::vector<VariableName> cv_vel(1, FlowModelTwoPhase::VELOCITY_VAPOR);

    std::string class_name = "OneDRusanov";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_VAPOR;
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "rusanov"), params);
  }

  if (fm.getPhaseInteraction())
  {
    std::vector<VariableName> cv_vel(1, FlowModelTwoPhase::VELOCITY_LIQUID);

    // volume fraction
    std::string class_name = "OneDRusanov";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_LIQUID;
    _m_sim.addKernel(class_name, Component::genName(comp_name, "beta", "rusanov"), params);
  }
}
