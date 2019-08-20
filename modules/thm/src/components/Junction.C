#include "Junction.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", Junction);

template <>
InputParameters
validParams<Junction>()
{
  InputParameters params = validParams<FlowJunction>();
  params.addParam<std::vector<Real>>("K", "Form loss coefficients");
  params.addRequiredParam<Real>("initial_p", "Initial pressure");
  params.addRequiredParam<Real>("initial_T", "Initial temperature");
  params.addParam<Real>("scaling_factor_s_junction", 1.0, "Scaling factor for junction entropy");
  return params;
}

Junction::Junction(const InputParameters & params)
  : FlowJunction(params),
    _entropy_var_name(genName(name(), "s")),
    _H_junction_uo_name(genName(name(), "H_uo")),
    _initial_p(getParam<Real>("initial_p")),
    _initial_T(getParam<Real>("initial_T")),
    _scaling_factor_s_junction(getParam<Real>("scaling_factor_s_junction"))
{
  if (isParamValid("K"))
  {
    _K = getParam<std::vector<Real>>("K");
    checkSizeEqualsNumberOfConnections<Real>("K");
  }
  else
  {
    _K = std::vector<Real>(getConnections().size(), 0);
  }
}

void
Junction::check() const
{
  FlowJunction::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);
}

void
Junction::addVariables()
{
  auto connected_subdomains = getConnectedSubdomainNames();

  const SinglePhaseFluidProperties & spfp =
      _sim.getUserObjectTempl<SinglePhaseFluidProperties>(_fp_name);

  const Real initial_rho = spfp.rho_from_p_T(_initial_p, _initial_T);
  const Real initial_e = spfp.e_from_p_rho(_initial_p, initial_rho);
  const Real initial_s = spfp.s_from_v_e(1. / initial_rho, initial_e);

  _sim.addVariable(true,
                   _entropy_var_name,
                   FEType(FIRST, SCALAR),
                   connected_subdomains,
                   _scaling_factor_s_junction);
  _sim.addConstantScalarIC(_entropy_var_name, initial_s);
}

void
Junction::addMooseObjects()
{
  // junction stagnation enthalpy
  {
    const std::string class_name = "JunctionStagnationEnthalpyUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
    params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
    params.set<MaterialPropertyName>("H") = FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY;
    params.set<std::vector<Real>>("normals") = _normals;
    _sim.addUserObject(class_name, _H_junction_uo_name, params);
  }

  // boundary fluxes
  {
    std::string class_name = "JunctionMassConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<Real>>("K") = _K;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
    params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("p") = {FlowModelSinglePhase::PRESSURE};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("s_junction") = {_entropy_var_name};
    params.set<UserObjectName>("H_junction_uo") = _H_junction_uo_name;
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addConstraint(class_name, genName(name(), "ced_rho"), params);
  }
  {
    std::string class_name = "JunctionMomentumConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<Real>>("K") = _K;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
    params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("p") = {FlowModelSinglePhase::PRESSURE};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("s_junction") = {_entropy_var_name};
    params.set<UserObjectName>("H_junction_uo") = _H_junction_uo_name;
    _sim.addConstraint(class_name, genName(name(), "ced_rhou"), params);
  }
  {
    std::string class_name = "JunctionEnergyConstraint";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<dof_id_type>>("nodes") = _nodes;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<Real>>("K") = _K;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
    params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("p") = {FlowModelSinglePhase::PRESSURE};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("s_junction") = {_entropy_var_name};
    params.set<UserObjectName>("H_junction_uo") = _H_junction_uo_name;
    _sim.addConstraint(class_name, genName(name(), "ced_rhoE"), params);
  }

  // mass balance constraint in junction
  {
    std::string class_name = "JunctionMassBalanceScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _entropy_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    _sim.addScalarKernel(class_name, genName(name(), "mass_balance"), params);
  }
}
