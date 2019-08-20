#include "VolumeJunctionOld.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", VolumeJunctionOld);

template <>
InputParameters
validParams<VolumeJunctionOld>()
{
  InputParameters params = validParams<VolumeJunctionOldBase>();
  return params;
}

VolumeJunctionOld::VolumeJunctionOld(const InputParameters & params)
  : VolumeJunctionOldBase(params),
    _rho_var_name(genName(name(), "rho")),
    _rhoe_var_name(genName(name(), "rhoE")),
    _vel_var_name(genName(name(), "vel")),
    _pressure_var_name(genName(name(), "p")),
    _energy_var_name(genName(name(), "energy")),
    _total_mfr_in_var_name(genName(name(), "total_mfr_in"))
{
}

void
VolumeJunctionOld::check() const
{
  VolumeJunctionOldBase::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);
}

void
VolumeJunctionOld::addVariables()
{
  // figure out delta H
  Real H_junction = _center(2);
  computeDeltaH(H_junction);
  // setup objects
  const SinglePhaseFluidProperties & spfp =
      _sim.getUserObjectTempl<SinglePhaseFluidProperties>(_fp_name);
  Real initial_rho = spfp.rho_from_p_T(_initial_p, _initial_T);
  Real initial_e = spfp.e_from_p_rho(_initial_p, initial_rho);

  auto connected_subdomains = getConnectedSubdomainNames();

  _sim.addVariable(
      true, _rho_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scale_factors[0]);
  _sim.addConstantScalarIC(_rho_var_name, initial_rho);
  _sim.addVariable(
      true, _rhoe_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scale_factors[1]);
  _sim.addConstantScalarIC(_rhoe_var_name, initial_rho * initial_e);
  _sim.addVariable(
      true, _vel_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scale_factors[2]);
  _sim.addConstantScalarIC(_vel_var_name, _initial_vel);
  // aux scalar vars
  _sim.addVariable(false, _pressure_var_name, FEType(FIRST, SCALAR));
  _sim.addConstantScalarIC(_pressure_var_name, _initial_p);
  _sim.addVariable(false, _energy_var_name, FEType(FIRST, SCALAR));
  _sim.addConstantScalarIC(_energy_var_name, initial_e);
  _sim.addVariable(false, _total_mfr_in_var_name, FEType(FIRST, SCALAR));
  _sim.addConstantScalarIC(_total_mfr_in_var_name, 0);
}

void
VolumeJunctionOld::addMooseObjects()
{
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_pressure(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_enthalpy(1, FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_v(1, FlowModelSinglePhase::SPECIFIC_VOLUME);
  std::vector<VariableName> cv_e(1, FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_junction_rho(1, _rho_var_name);
  std::vector<VariableName> cv_junction_rhoe(1, _rhoe_var_name);
  std::vector<VariableName> cv_junction_vel(1, _vel_var_name);
  std::vector<VariableName> cv_junction_pressure(1, _pressure_var_name);
  std::vector<VariableName> cv_junction_energy(1, _energy_var_name);

  {
    std::string class_name = "TotalMassFlowRateIntoJunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _total_mfr_in_var_name;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    _sim.addAuxScalarKernel(class_name, genName(name(), "total_mfr_in"), params);
  }
  for (unsigned int i = 0; i < _boundary_names.size(); ++i)
  {
    // BCs
    {
      std::string class_name = "OneDMassFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
      _sim.addBoundaryCondition(class_name, genName("bc", _boundary_names[i], "rho"), params);
    }
    {
      std::string class_name = "VolumeJunctionOldBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<MooseEnum>("eqn_name") = FlowModel::getFlowEquationType("MOMENTUM");
      params.set<Real>("K") = _k_coeffs[i];
      params.set<Real>("K_reverse") = _kr_coeffs[i];
      params.set<Real>("ref_area") = _ref_area;
      params.set<Real>("deltaH") = _deltaH[i];
      params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
      params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
      params.set<std::vector<VariableName>>("rho") = cv_rho;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("rho_junction") = cv_junction_rho;
      params.set<std::vector<VariableName>>("rhoe_junction") = cv_junction_rhoe;
      params.set<std::vector<VariableName>>("vel_junction") = cv_junction_vel;
      params.set<std::vector<VariableName>>("p_junction") = cv_junction_pressure;
      params.set<UserObjectName>("fp") = _fp_name;

      params.set<Real>("gravity_magnitude") = _gravity_magnitude;

      std::string nm = genName("bc", _boundary_names[i], "rhou");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, nm, "A_ref", "ref_area");
    }
    {
      std::string class_name = "VolumeJunctionOldBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<MooseEnum>("eqn_name") = FlowModel::getFlowEquationType("ENERGY");
      params.set<Real>("K") = _k_coeffs[i];
      params.set<Real>("K_reverse") = _kr_coeffs[i];
      params.set<Real>("ref_area") = _ref_area;
      params.set<Real>("deltaH") = _deltaH[i];
      params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
      params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
      params.set<std::vector<VariableName>>("rho") = cv_rho;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("rho_junction") = cv_junction_rho;
      params.set<std::vector<VariableName>>("rhoe_junction") = cv_junction_rhoe;
      params.set<std::vector<VariableName>>("vel_junction") = cv_junction_vel;
      params.set<std::vector<VariableName>>("p_junction") = cv_junction_pressure;
      params.set<UserObjectName>("fp") = _fp_name;
      params.set<Real>("gravity_magnitude") = _gravity_magnitude;

      std::string nm = genName("bc", _boundary_names[i], "rhoE");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, nm, "A_ref", "ref_area");
    }
  }
  // add constraint for non-linear scalar
  {
    std::string class_name = "ODECoefTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rho_var_name;
    params.set<Real>("coef") = _volume;
    std::string nm = genName(name(), _rho_var_name, "td");
    _sim.addScalarKernel(class_name, nm, params);
    connectObject(params, nm, "volume", "coef");
  }
  {
    InputParameters params = _factory.getValidParams("MassBalanceScalarKernel");
    params.set<NonlinearVariableName>("variable") = _rho_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    std::string nm = genName(name(), "mass_balance");
    _sim.addScalarKernel("MassBalanceScalarKernel", nm, params);
  }

  {
    std::string class_name = "ODECoefTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoe_var_name;
    params.set<Real>("coef") = _volume;
    std::string nm = genName(name(), _rhoe_var_name, "td");
    _sim.addScalarKernel(class_name, nm, params);
    connectObject(params, nm, "volume", "coef");
  }
  {
    std::string class_name = "EnergyBalanceScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoe_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<UserObjectName>("fp") = _fp_name;
    // coupling variables
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<std::vector<VariableName>>("e") = cv_e;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("p") = cv_pressure;
    params.set<std::vector<VariableName>>("A") = cv_area;
    std::string nm = genName(name(), _rhoe_var_name, "st");
    _sim.addScalarKernel(class_name, nm, params);
  }
  {
    std::string class_name = "MomentumBalanceScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _vel_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<Real>("ref_area") = _ref_area;
    params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("junction_rho") = cv_junction_rho;
    std::string nm = genName(name(), "energy_balance", _vel_var_name);
    _sim.addScalarKernel(class_name, nm, params);
    connectObject(params, nm, "A_ref", "ref_area");
  }
  {
    std::string class_name = "VolumeJunctionOldPressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _pressure_var_name;
    params.set<UserObjectName>("fp") = _fp_name;
    // coupling variables
    params.set<std::vector<VariableName>>("junction_rho") = cv_junction_rho;
    params.set<std::vector<VariableName>>("junction_rhoe") = cv_junction_rhoe;
    _sim.addAuxScalarKernel(class_name, genName(name(), "p_aux"), params);
  }

  {
    std::string class_name = "QuotientScalarAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _energy_var_name;
    // coupling variables
    params.set<std::vector<VariableName>>("numerator") = cv_junction_rhoe;
    params.set<std::vector<VariableName>>("denominator") = cv_junction_rho;
    _sim.addAuxScalarKernel(class_name, genName(name(), "energy_aux"), params);
  }
}
