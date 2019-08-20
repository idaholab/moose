#include "OldJunction.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", OldJunction);

template <>
InputParameters
validParams<OldJunction>()
{
  InputParameters params = validParams<JunctionWithLossesBase>();
  params.addParam<Real>("initial_p", 1e5, "Initial pressure of this junction");
  params.addParam<Real>("initial_T", 300, "Initial temperature of this junction");
  params.addParam<Real>(
      "initial_alpha_vapor", 1., "Initial vapor volume fraction in the OldJunction");
  std::vector<Real> sf(2, 1.e0);
  params.addParam<std::vector<Real>>(
      "scaling_factors", sf, "Scaling factors for variables: pressure, energy");
  return params;
}

OldJunction::OldJunction(const InputParameters & params)
  : JunctionWithLossesBase(params),
    _pressure_var_name(genName(name(), "p")),
    _energy_var_name(genName(name(), "energy")),
    _initial_P(getParam<Real>("initial_p")),
    _initial_void_fraction(getParam<Real>("initial_alpha_vapor")),
    _total_mfr_in_var_name(genName(name(), "tmfri")),
    _total_int_energy_rate_in_var_name(genName(name(), "ntei")),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factors"))
{
}

void
OldJunction::check() const
{
  JunctionWithLossesBase::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);

  checkSizeEqualsValue<Real>("scaling_factors", 2);
}

void
OldJunction::addVariables()
{
  auto connected_subdomains = getConnectedSubdomainNames();

  _sim.addVariable(
      true, _pressure_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factors[0]);
  _sim.addConstantScalarIC(_pressure_var_name, _initial_P);

  const SinglePhaseFluidProperties & spfp =
      _sim.getUserObjectTempl<SinglePhaseFluidProperties>(_fp_name);
  Real initial_T = getParam<Real>("initial_T");
  Real initial_rho = spfp.rho_from_p_T(_initial_P, initial_T);
  Real initial_e = spfp.e_from_p_rho(_initial_P, initial_rho);

  _sim.addVariable(
      true, _energy_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factors[1]);
  _sim.addConstantScalarIC(_energy_var_name, initial_e);

  _sim.addVariable(false, _total_mfr_in_var_name, FEType(FIRST, SCALAR));
  _sim.addConstantScalarIC(_total_mfr_in_var_name, 0);

  _sim.addVariable(false, _total_int_energy_rate_in_var_name, FEType(FIRST, SCALAR));
  _sim.addConstantScalarIC(_total_int_energy_rate_in_var_name, 0);
}

void
OldJunction::addMooseObjects()
{
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_pressure(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_enthalpy(1, FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_area(1, FlowModelSinglePhase::AREA);
  std::vector<VariableName> cv_junction_pressure(1, _pressure_var_name);
  std::vector<VariableName> cv_junction_energy(1, _energy_var_name);

  std::vector<OutputName> outputs = _sim.getOutputsVector("none");

  const SinglePhaseFluidProperties & spfp =
      _sim.getUserObjectTempl<SinglePhaseFluidProperties>(_fp_name);
  Real initial_T = getParam<Real>("initial_T");
  Real initial_rho = spfp.rho_from_p_T(_initial_P, initial_T);

  {
    std::string class_name = "TotalMassFlowRateIntoJunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _total_mfr_in_var_name;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    _sim.addAuxScalarKernel(class_name, genName(name(), "total_mfr_in"), params);
  }

  {
    std::string class_name = "TotalInternalEnergyRateIntoJunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _total_int_energy_rate_in_var_name;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    _sim.addAuxScalarKernel(class_name, genName(name(), "total_int_energy_rate_in"), params);
  }

  {
    std::string class_name = "MassFreeConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    _sim.addConstraint(class_name, genName(name(), "ced_rho"), params);
  }
  {
    std::string class_name = "OldJunctionMomentumConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<Real>>("K") = _k_coeffs;
    params.set<std::vector<Real>>("K_reverse") = _kr_coeffs;
    params.set<Real>("ref_area") = _ref_area;
    params.set<Real>("initial_rho") = initial_rho;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
    params.set<std::vector<VariableName>>("total_int_energy_rate_in") = {
        _total_int_energy_rate_in_var_name};
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("p_junction") = cv_junction_pressure;
    _sim.addConstraint(class_name, genName(name(), "ced_rhou"), params);
  }

  {
    std::string class_name = "OldJunctionEnergyConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<Real>>("K") = _k_coeffs;
    params.set<std::vector<Real>>("K_reverse") = _kr_coeffs;
    params.set<Real>("ref_area") = _ref_area;
    params.set<Real>("initial_rho") = initial_rho;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
    params.set<std::vector<VariableName>>("total_int_energy_rate_in") = {
        _total_int_energy_rate_in_var_name};
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("p_junction") = cv_junction_pressure;
    params.set<std::vector<VariableName>>("energy_junction") = cv_junction_energy;
    _sim.addConstraint(class_name, genName(name(), "ced_rhoE"), params);
  }

  // add constraints
  {
    std::string class_name = "OldJunctionMassBalanceScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _pressure_var_name;

    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;

    _sim.addScalarKernel(class_name, genName(name(), "mass_balance"), params);
  }
  {
    std::string class_name = "EnergyScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _energy_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;

    params.set<std::vector<VariableName>>("total_mfr_in") = {_total_mfr_in_var_name};
    params.set<std::vector<VariableName>>("total_int_energy_rate_in") = {
        _total_int_energy_rate_in_var_name};
    params.set<std::vector<Real>>("normals") = _normals;

    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("A") = cv_area;

    _sim.addScalarKernel(class_name, genName(name(), "energy_ced"), params);
  }
}
