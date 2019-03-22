#include "InletStagnationEnthalpyMomentum.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"

registerMooseObject("THMApp", InletStagnationEnthalpyMomentum);

template <>
InputParameters
validParams<InletStagnationEnthalpyMomentum>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("rhou", "Prescribed momentum");
  params.addParam<Real>("H", "Prescribed specific total enthalpy");
  // 2-phase
  params.addParam<Real>("rhou_liquid", "Prescribed momentum for liquid");
  params.addParam<Real>("rhou_vapor", "Prescribed momentum for vapor");
  params.addParam<Real>("H_liquid", "Prescribed specific total enthalpy for liquid");
  params.addParam<Real>("H_vapor", "Prescribed specific total enthalpy for vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletStagnationEnthalpyMomentum::InletStagnationEnthalpyMomentum(const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationEnthalpyMomentum::check() const
{
  FlowBoundary::check();

  // Input type checking
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    for (auto p : {"H", "rhou"})
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
    }

    for (auto p : {"H_liquid", "rhou_liquid", "H_vapor", "rhou_vapor"})
      check2PhaseRequiredParameter(p);

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
InletStagnationEnthalpyMomentum::setup1Phase()
{
  Real H = getParam<Real>("H");
  Real rhou = getParam<Real>("rhou");

  {
    std::string class_name = "OneDMassHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou");
  }

  {
    std::string class_name = "OneDMomentumHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou");
  }

  {
    std::string class_name = "OneDEnergyHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<Real>("H") = H;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou");
    connectObject(params, "", nm, "H");
  }
}

void
InletStagnationEnthalpyMomentum::setup2Phase()
{
  Real alpha = getParam<Real>("alpha_vapor");
  Real H_liquid = getParam<Real>("H_liquid");
  Real H_vapor = getParam<Real>("H_vapor");
  Real rhou_liquid = getParam<Real>("rhou_liquid");
  Real rhou_vapor = getParam<Real>("rhou_vapor");
  std::string nm;

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
    nm = genName(name(), "vf_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "alpha_vapor", "alpha");
  }

  {
    std::string class_name = "OneDMassHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou_liquid;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    nm = genName(name(), "arhoA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_liquid", "rhou");
  }
  {
    std::string class_name = "OneD7EqnAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("is_liquid") = true;
    params.set<Real>("value") = rhou_liquid;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("alpha") = {FlowModelTwoPhase::VOLUME_FRACTION_LIQUID};
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};

    nm = genName(name(), "arhouA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_liquid", "value");
  }

  {
    std::string class_name = "OneDEnergyHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou_liquid;
    params.set<Real>("H") = H_liquid;
    // coupling
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    nm = genName(name(), "arhoEA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_liquid", "rhou");
    connectObject(params, "", nm, "H_liquid", "H");
  }

  {
    std::string class_name = "OneDMassHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou_vapor;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    nm = genName(name(), "arhoA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_vapor", "rhou");
  }
  {
    std::string class_name = "OneD7EqnAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("is_liquid") = false;
    params.set<Real>("value") = rhou_vapor;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("alpha") = {FlowModelTwoPhase::VOLUME_FRACTION_VAPOR};
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};

    nm = genName(name(), "arhouA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_vapor", "value");
  }

  {
    std::string class_name = "OneDEnergyHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou_vapor;
    params.set<Real>("H") = H_vapor;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    nm = genName(name(), "arhoEA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "rhou_vapor", "rhou");
    connectObject(params, "", nm, "H_vapor", "H");
  }
}

void
InletStagnationEnthalpyMomentum::setup2PhaseNCG()
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
InletStagnationEnthalpyMomentum::addMooseObjects()
{
  // apply logic for parameters with one- and two-phase variants
  const bool is_two_phase =
      _flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "rhou", {"rhou_liquid", "rhou_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "H", {"H_liquid", "H_vapor"}, *this);

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    setup1Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE)
    setup2Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    setup2PhaseNCG();
}
