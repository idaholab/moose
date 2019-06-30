#include "Outlet.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"

registerMooseObject("THMApp", Outlet);

template <>
InputParameters
validParams<Outlet>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addParam<bool>(
      "reversible",
      false,
      "true for reversible outlet boundary conditions (works only for 7-eqn model)");
  // single phase
  params.addParam<Real>("p", "prescribed pressure");
  // two phase
  params.addParam<Real>("p_liquid", "Prescribed pressure for the liquid phase");
  params.addParam<Real>("p_vapor", "Prescribed pressure for the vapor phase");

  params.addParam<bool>(
      "legacy", false, "Use the old version of the BC (violating characteristic theory) or not.");

  return params;
}

Outlet::Outlet(const InputParameters & params)
  : FlowBoundary(params),
    _reversible(getParam<bool>("reversible")),
    _legacy(getParam<bool>("legacy"))
{
}

void
Outlet::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    check1PhaseRequiredParameter("p");
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    check2PhaseRequiredParameter("p_liquid");
    check2PhaseRequiredParameter("p_vapor");
  }
  else
    logModelNotImplementedError(_flow_model_id);

  if (_flow_model_id == THM::FM_TWO_PHASE_NCG && _spatial_discretization == FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
Outlet::add3EqnStaticPBC()
{
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_v(1, FlowModelSinglePhase::SPECIFIC_VOLUME);
  std::vector<VariableName> cv_e(1, FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_p(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_temperature(1, FlowModelSinglePhase::TEMPERATURE);
  std::vector<VariableName> cv_enthalpy(1, FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoA"), params);
  }

  {
    std::string class_name = "OneDMomentumStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "bc_rhou");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }

  {
    std::string class_name = "OneDEnergyStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<bool>("is_liquid") = true;
    params.set<Real>("p_in") = p;
    params.set<Real>("normal") = _normal;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("e") = cv_e;
    params.set<std::vector<VariableName>>("v") = cv_v;
    std::string nm = genName(name(), "bc_rhoE");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureSupersonicBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<bool>("is_liquid") = true;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("H") = cv_enthalpy;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("e") = cv_e;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    params.set<MaterialPropertyName>("c") = FlowModelSinglePhase::SOUND_SPEED;
    std::string nm = genName(name(), "bc_rhoE2");
    _sim.addBoundaryCondition(class_name, nm, params);
  }
}

void
Outlet::add3EqnStaticPReverseBC()
{
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_p(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_temperature(1, FlowModelSinglePhase::TEMPERATURE);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneD7EqnMassStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("T") = cv_temperature;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    std::string nm = genName(name(), "rhoA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
  {
    std::string class_name = "OneDMomentumStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("p") = p;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("T") = cv_temperature;
    std::string nm = genName(name(), "rhouA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
  {
    std::string class_name = "OneD7EqnEnergyStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("T") = cv_temperature;
    std::string nm = genName(name(), "rhoEA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
}

void
Outlet::add3EqnStaticPBCLegacy()
{
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_p(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_temperature(1, FlowModelSinglePhase::TEMPERATURE);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoA"), params);
  }

  {
    std::string class_name = "OneDMomentumStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "bc_rhou");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }

  {
    std::string class_name = "OneDEnergyStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "bc_rhoE");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }
}

void
Outlet::addMooseObjects3EqnRDG()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux3EqnGhostPressure";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p") = getParam<Real>("p");
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
    connectObject(params, boundary_flux_name, "p");
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
Outlet::add7EqnStaticPReverseBC()
{
  const TwoPhaseFluidProperties & tpfp = _sim.getUserObjectTempl<TwoPhaseFluidProperties>(_fp_name);
  const UserObjectName & fp_liquid = tpfp.getLiquidName();
  const UserObjectName & fp_vapor = tpfp.getVaporName();

  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_rho_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_temperature_liquid(1, FlowModelTwoPhase::TEMPERATURE_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_rho_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_temperature_vapor(1, FlowModelTwoPhase::TEMPERATURE_VAPOR);

  Real p_liquid = getParam<Real>("p_liquid");
  Real p_vapor = getParam<Real>("p_vapor");

  // Liquid phase
  {
    std::string class_name = "OneD7EqnMassStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p_liquid;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("T") = cv_temperature_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    std::string nm = genName(name(), "arhoA_liquid_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p");
  }
  {
    std::string class_name = "OneDMomentumStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("p") = p_liquid;
    params.set<UserObjectName>("fp") = fp_liquid;
    params.set<bool>("is_liquid") = true;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("T") = cv_temperature_liquid;
    std::string nm = genName(name(), "arhouA_liquid_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p");
  }
  {
    std::string class_name = "OneD7EqnEnergyStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p_liquid;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("T") = cv_temperature_liquid;
    std::string nm = genName(name(), "arhoEA_liquid_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p");
  }
  // Vapor phase
  {
    std::string class_name = "OneD7EqnMassStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p_vapor;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_vapor;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("T") = cv_temperature_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    std::string nm = genName(name(), "arhoA_vapor_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p");
  }
  {
    std::string class_name = "OneDMomentumStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("p") = p_vapor;
    params.set<UserObjectName>("fp") = fp_vapor;
    params.set<bool>("is_liquid") = false;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("T") = cv_temperature_vapor;
    std::string nm = genName(name(), "arhouA_vapor_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p");
  }
  {
    std::string class_name = "OneD7EqnEnergyStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p_vapor;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_vapor;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("T") = cv_temperature_vapor;
    std::string nm = genName(name(), "arhoEA_vapor_rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p");
  }
}

void
Outlet::add7EqnStaticPBC()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_rho_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_e_liquid(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_LIQUID);
  std::vector<VariableName> cv_v_liquid(1, FlowModelTwoPhase::SPECIFIC_VOLUME_LIQUID);
  std::vector<VariableName> cv_p_liquid(1, FlowModelTwoPhase::PRESSURE_LIQUID);
  std::vector<VariableName> cv_temperature_liquid(1, FlowModelTwoPhase::TEMPERATURE_LIQUID);
  std::vector<VariableName> cv_enthalpy_liquid(1,
                                               FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_rho_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_e_vapor(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_VAPOR);
  std::vector<VariableName> cv_v_vapor(1, FlowModelTwoPhase::SPECIFIC_VOLUME_VAPOR);
  std::vector<VariableName> cv_p_vapor(1, FlowModelTwoPhase::PRESSURE_VAPOR);
  std::vector<VariableName> cv_temperature_vapor(1, FlowModelTwoPhase::TEMPERATURE_VAPOR);
  std::vector<VariableName> cv_enthalpy_vapor(1, FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_VAPOR);

  const TwoPhaseFluidProperties & tpfp = _sim.getUserObjectTempl<TwoPhaseFluidProperties>(_fp_name);
  const UserObjectName & fp_liquid = tpfp.getLiquidName();
  const UserObjectName & fp_vapor = tpfp.getVaporName();

  Real p_liquid = getParam<Real>("p_liquid");
  Real p_vapor = getParam<Real>("p_vapor");

  // liquid phase
  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    std::string nm = genName(name(), "arhoA_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
  }
  {
    std::string class_name = "OneDMomentumStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "arhouA_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fp") = fp_liquid;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<bool>("is_liquid") = true;
    params.set<Real>("p_in") = p_liquid;
    params.set<Real>("normal") = _normal;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rho") = cv_rho_liquid;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("rhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("e") = cv_e_liquid;
    params.set<std::vector<VariableName>>("v") = cv_v_liquid;
    std::string nm = genName(name(), "arhoEA_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureSupersonicBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fp") = fp_liquid;
    params.set<bool>("is_liquid") = true;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("H") = cv_enthalpy_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("e") = cv_e_liquid;
    params.set<std::vector<VariableName>>("v") = cv_v_liquid;
    params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_LIQUID;
    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_LIQUID;
    std::string nm = genName(name(), "arhoEA_liquid2");
    _sim.addBoundaryCondition(class_name, nm, params);
  }
  // vapor phase
  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    _sim.addBoundaryCondition(class_name, genName(name(), "arhoA_vapor"), params);
  }
  {
    std::string class_name = "OneDMomentumStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "arhouA_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fp") = fp_vapor;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<bool>("is_liquid") = false;
    params.set<Real>("p_in") = p_vapor;
    params.set<Real>("normal") = _normal;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rho") = cv_rho_vapor;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("rhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("e") = cv_e_vapor;
    params.set<std::vector<VariableName>>("v") = cv_v_vapor;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    std::string nm = genName(name(), "arhoEA_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureSupersonicBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fp") = fp_vapor;
    params.set<bool>("is_liquid") = false;
    // coupling
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("H") = cv_enthalpy_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("e") = cv_e_vapor;
    params.set<std::vector<VariableName>>("v") = cv_v_vapor;
    params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_VAPOR;
    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_VAPOR;
    std::string nm = genName(name(), "arhoEA_vapor2");
    _sim.addBoundaryCondition(class_name, nm, params);
  }
}

void
Outlet::add7EqnStaticPBCLegacy()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_rho_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_temperature_liquid(1, FlowModelTwoPhase::TEMPERATURE_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_rho_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_temperature_vapor(1, FlowModelTwoPhase::TEMPERATURE_VAPOR);

  Real p_liquid = getParam<Real>("p_liquid");
  Real p_vapor = getParam<Real>("p_vapor");

  // liquid phase
  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    _sim.addBoundaryCondition(class_name, genName(name(), "arhoA_liquid"), params);
  }
  {
    std::string class_name = "OneDMomentumStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "arhouA_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    std::string nm = genName(name(), "arhoEA_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_liquid", "p_in");
  }
  // vapor phase
  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    _sim.addBoundaryCondition(class_name, genName(name(), "arhoA_vapor"), params);
  }
  {
    std::string class_name = "OneDMomentumStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("rhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("rhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), "arhouA_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    std::string nm = genName(name(), "arhoEA_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p_vapor", "p_in");
  }
}

void
Outlet::addNCGStaticPBC()
{
  const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
  const std::vector<VariableName> & vars = fm.getNCGSolutionVars();

  for (std::size_t i = 0; i < vars.size(); i++)
  {
    {
      std::string class_name = "OneDMassNCGFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = vars[i];
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<std::vector<VariableName>>("arhoA") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
      params.set<std::vector<VariableName>>("arhouA") = {FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
      params.set<std::vector<VariableName>>("vel") = {FlowModelTwoPhase::VELOCITY_VAPOR};
      _sim.addBoundaryCondition(class_name, genName(name(), "aXrhoA_vapor_bc"), params);
    }
  }
}

void
Outlet::addMooseObjects7EqnRDG()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux7EqnGhostPressure";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p_liquid") = getParam<Real>("p_liquid");
    params.set<Real>("p_vapor") = getParam<Real>("p_vapor");
    params.set<UserObjectName>("fp_2phase") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
  }

  // BCs
  addWeakBC7Eqn(boundary_flux_name);
}

void
Outlet::addMooseObjects()
{
  // apply logic for parameters with one- and two-phase variants
  const bool is_two_phase =
      _flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "p", {"p_liquid", "p_vapor"}, *this);

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    if (_spatial_discretization == FlowModel::CG)
    {
      if (_legacy)
        add3EqnStaticPBCLegacy();
      else
        add3EqnStaticPBC();
      if (_reversible)
        add3EqnStaticPReverseBC();
    }
    else if (_spatial_discretization == FlowModel::rDG)
      addMooseObjects3EqnRDG();
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    if (_spatial_discretization == FlowModel::CG)
    {
      if (_legacy)
        add7EqnStaticPBCLegacy();
      else
        add7EqnStaticPBC();
      if (_reversible)
        add7EqnStaticPReverseBC();

      if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
        addNCGStaticPBC();
    }
    else if (_spatial_discretization == FlowModel::rDG)
      addMooseObjects7EqnRDG();
  }
}
