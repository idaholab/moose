#include "SUPG.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "Component.h"
#include "FlowChannelBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

const std::string SUPG::DELTA = "SUPG_delta";
const std::string SUPG::RESIDUAL = "SUPG_R";
const std::string SUPG::MATRIX = "SUPG_A";
const std::string SUPG::COLUMNS = "SUPG_y";
const std::string SUPG::DUDX = "SUPG_dUdx";
const std::string SUPG::DADU = "SUPG_dAdU";

registerMooseObject("THMApp", SUPG);

template <>
InputParameters
validParams<SUPG>()
{
  InputParameters params = validParams<StabilizationSettings>();
  // Shock capturing is "extra" artificial diffusion which may be required
  // when strong discontinuities are present in the flow
  params.addParam<bool>("shock_capturing", false, "Use shock capturing or not");
  return params;
}

SUPG::SUPG(const InputParameters & parameters)
  : StabilizationSettings(parameters), _shock_capturing(getParam<bool>("shock_capturing"))
{
}

void
SUPG::addVariables(FlowModel &, SubdomainID) const
{
  // no extra variables needed for SUPG
}

void
SUPG::initMooseObjects(FlowModel &)
{
}

void
SUPG::addMooseObjects(FlowModel & fm, InputParameters & pars) const
{
  if (dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL)
    mooseError("SUPG is not implemented with 2-phase flow model.");

  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  FlowChannelBase * fch = dynamic_cast<FlowChannelBase *>(comp);
  if (fch == nullptr)
    mooseError(
        name(),
        ": Trying to use SUPG stabilization with a component that is not a FlowChannelBase.");

  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  // coupling vectors
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_enthalpy(1, FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_temperature(1, FlowModelSinglePhase::TEMPERATURE);
  std::vector<VariableName> cv_T_wall(1, FlowModelSinglePhase::TEMPERATURE_WALL);
  std::vector<VariableName> cv_P_hf(1, FlowModelSinglePhase::HEAT_FLUX_PERIMETER);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  // SUPG, this should be added after friction and heat transfer materials
  {
    std::string class_name = "SUPGMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<UserObjectName>("fp") = pars.get<UserObjectName>("fp");
    params.set<MaterialPropertyName>("f_D") = "f_D";
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<MaterialPropertyName>("D_h") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
    params.set<std::vector<VariableName>>("H") = cv_enthalpy;
    params.set<std::vector<VariableName>>("T") = cv_temperature;
    params.set<std::vector<VariableName>>("P_hf") = cv_P_hf;
    if (fch->getNumberOfHeatTransferConnections() > 0)
    {
      params.set<MaterialPropertyName>("Hw") = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
      params.set<MaterialPropertyName>("T_wall") = FlowModel::TEMPERATURE_WALL;
    }
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    params.set<RealVectorValue>("gravity_vector") = pars.get<RealVectorValue>("gravity_vector");

    if (_shock_capturing)
      params.set<unsigned>("n_iterations_before_freezing_delta") = 1;

    _m_sim.addMaterial(class_name, Component::genName(comp_name, "r7_material"), params);
  }

  std::vector<std::string> vars = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  // SUPG kernels
  for (unsigned int i = 0; i < vars.size(); i++)
  {
    std::string class_name = "OneDSUPG";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<unsigned int>("component") = i;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    params.set<MaterialPropertyName>("SUPG_delta") = DELTA;
    params.set<MaterialPropertyName>("SUPG_R") = RESIDUAL;
    params.set<MaterialPropertyName>("SUPG_A") = MATRIX;
    params.set<MaterialPropertyName>("SUPG_y") = COLUMNS;
    params.set<MaterialPropertyName>("SUPG_dUdx") = DUDX;
    params.set<MaterialPropertyName>("SUPG_dAdU") = DADU;
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "supg"), params);
  }

  if (_shock_capturing)
  {
    mooseError("Shock capturing not implemented for variable-area model.");

    for (unsigned int i = 0; i < vars.size(); i++)
    {
      std::string class_name = "OneDShockCapturing";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = vars[i];
      params.set<std::vector<SubdomainName>>("block") = blocks;
      params.set<UserObjectName>("fp") = pars.get<UserObjectName>("fp");
      params.set<MaterialPropertyName>("SUPG_delta") = DELTA;
      params.set<MaterialPropertyName>("SUPG_R") = RESIDUAL;
      params.set<MaterialPropertyName>("SUPG_A") = MATRIX;
      params.set<MaterialPropertyName>("SUPG_y") = COLUMNS;
      params.set<MaterialPropertyName>("SUPG_dUdx") = DUDX;
      params.set<MaterialPropertyName>("SUPG_dAdU") = DADU;
      _m_sim.addKernel(class_name, Component::genName(comp_name, vars[i], "sc"), params);
    }
  }
}
