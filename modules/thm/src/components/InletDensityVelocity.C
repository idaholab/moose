#include "InletDensityVelocity.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"

registerMooseObject("THMApp", InletDensityVelocity);

template <>
InputParameters
validParams<InletDensityVelocity>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("rho", "Prescribed density");
  params.addParam<Real>("vel", "Prescribed velocity");
  // 2-phase
  params.addParam<Real>("rho_liquid", "Prescribed density of liquid");
  params.addParam<Real>("vel_liquid", "Prescribed velocity of liquid");
  params.addParam<Real>("rho_vapor", "Prescribed density of vapor");
  params.addParam<Real>("vel_vapor", "Prescribed velocity of vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletDensityVelocity::InletDensityVelocity(const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletDensityVelocity::check() const
{
  FlowBoundary::check();

  // Input type checking
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    for (auto p : {"rho", "vel"})
      check1PhaseRequiredParameter(p);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    auto fm = dynamic_cast<const FlowModelTwoPhase *>(_flow_model.get());
    if (fm != nullptr)
    {
      bool phase_interaction = fm->getPhaseInteraction();
      if (phase_interaction)
        check7EqnRequiredParameter("alpha_vapor");
      for (auto p : {"rho_liquid", "vel_liquid", "rho_vapor", "vel_vapor"})
        check2PhaseRequiredParameter(p);
    }

    if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    {
      check7EqnNCGRequiredParameter("x_ncgs");

      if (isParamValid("x_ncgs"))
      {
        auto fm_ncg = dynamic_cast<const FlowModelTwoPhaseNCG *>(_flow_model.get());
        if (fm_ncg != nullptr)
          checkSizeEqualsValue<Real>("x_ncgs", fm_ncg->getNCGSolutionVars().size());
      }
    }
  }
  else
    logModelNotImplementedError(_flow_model_id);

  if (FlowModel::getSpatialDiscretizationType() == FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(FlowModel::getSpatialDiscretizationType());
}

void
InletDensityVelocity::setup1Phase()
{
  // coupling vectors
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);

  Real rho_in = getParam<Real>("rho");
  Real u_in = getParam<Real>("vel");

  {
    std::string class_name = "OneDAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("value") = rho_in;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho", "value");
  }
  {
    std::string class_name = "OneDMomentumDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_in;
    params.set<Real>("vel") = u_in;
    params.set<UserObjectName>("fp") = _fp_name;
    // coupling
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho");
    connectObject(params, nm, "vel", "vel");
  }
  {
    std::string class_name = "OneDEnergyDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_in;
    params.set<Real>("vel") = u_in;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<UserObjectName>("fp") = _fp_name;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho");
    connectObject(params, nm, "vel", "vel");
  }
}

void
InletDensityVelocity::setup2Phase()
{
  // coupling vectors
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);

  Real alpha = getParam<Real>("alpha_vapor");
  Real rho_liquid = getParam<Real>("rho_liquid");
  Real rho_vapor = getParam<Real>("rho_vapor");
  Real vel_liquid = getParam<Real>("vel_liquid");
  Real vel_vapor = getParam<Real>("vel_vapor");
  const TwoPhaseFluidProperties & tpfp = _sim.getUserObject<TwoPhaseFluidProperties>(_fp_name);
  const UserObjectName & fp_liquid = tpfp.getLiquidName();
  const UserObjectName & fp_vapor = tpfp.getVaporName();

  const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
  bool phase_interaction = fm.getPhaseInteraction();

  // BC for beta
  if (phase_interaction)
  {
    std::string class_name = "OneDVolumeFractionVaporBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("alpha") = alpha;
    std::string nm = genName(name(), "vf_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "alpha_vapor", "alpha");
  }

  // BC for liquid phase: mass, momentum, and energy
  {
    std::string class_name = "OneD7EqnAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("is_liquid") = true;
    params.set<Real>("value") = rho_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    std::string nm = genName(name(), "arhoA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_liquid", "value");
  }
  {
    std::string class_name = "OneDMomentumDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_liquid;
    params.set<Real>("vel") = vel_liquid;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_liquid;
    std::string nm = genName(name(), "arhouA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_liquid", "rho");
    connectObject(params, nm, "vel_liquid", "vel");
  }
  {
    std::string class_name = "OneDEnergyDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_liquid;
    params.set<Real>("vel") = vel_liquid;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_liquid;
    std::string nm = genName(name(), "arhoEA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_liquid", "rho");
    connectObject(params, nm, "vel_liquid", "vel");
  }

  // BC for vapor phase: mass, momentum, and energy
  {
    std::string class_name = "OneD7EqnAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("is_liquid") = false;
    params.set<Real>("value") = rho_vapor;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    std::string nm = genName(name(), "arhoA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_vapor", "value");
  }
  {
    std::string class_name = "OneDMomentumDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_vapor;
    params.set<Real>("vel") = vel_vapor;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_vapor;
    std::string nm = genName(name(), "arhouA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_vapor", "rho");
    connectObject(params, nm, "vel_vapor", "vel");
  }
  {
    std::string class_name = "OneDEnergyDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_vapor;
    params.set<Real>("vel") = vel_vapor;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_vapor;
    std::string nm = genName(name(), "arhoEA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho_vapor", "rho");
    connectObject(params, nm, "vel_vapor", "vel");
  }
}

void
InletDensityVelocity::setup2PhaseNCG()
{
  setup2Phase();

  const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
  const std::vector<VariableName> & vars = fm.getNCGSolutionVars();
  const std::vector<Real> & x_ncgs = getParam<std::vector<Real>>("x_ncgs");

  for (std::size_t i = 0; i < vars.size(); i++)
  {
    {
      std::string class_name = "OneDMassNCGSpecifiedMassFractionBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = vars[i];
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<Real>("x_ncg") = x_ncgs[i];
      params.set<std::vector<VariableName>>("arhouA_vapor") = {
          FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
      _sim.addBoundaryCondition(class_name, genName(name(), "aXrhoA_vapor_bc"), params);
    }
  }
}

void
InletDensityVelocity::addMooseObjects()
{
  // apply logic for parameters with one- and two-phase variants
  const bool is_two_phase =
      _flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "rho", {"rho_liquid", "rho_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "vel", {"vel_liquid", "vel_vapor"}, *this);

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    setup1Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE)
    setup2Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    setup2PhaseNCG();
}
