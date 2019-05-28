#include "FreeBoundary.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", FreeBoundary);

template <>
InputParameters
validParams<FreeBoundary>()
{
  InputParameters params = validParams<FlowBoundary>();
  return params;
}

FreeBoundary::FreeBoundary(const InputParameters & parameters) : FlowBoundary(parameters) {}

void
FreeBoundary::init()
{
  FlowBoundary::init();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    _is_two_phase = false;
    _phase_suffix = {""};
    _arhoA_name = {FlowModelSinglePhase::RHOA};
    _arhouA_name = {FlowModelSinglePhase::RHOUA};
    _arhoEA_name = {FlowModelSinglePhase::RHOEA};
    _velocity_name = {FlowModelSinglePhase::VELOCITY};
    _enthalpy_name = {FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY};
    _pressure_name = {FlowModelSinglePhase::PRESSURE};
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    _is_two_phase = true;
    _is_liquid = {true, false};
    _phase_suffix = {"_liquid", "_vapor"};
    _arhoA_name = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
    _arhouA_name = {FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
    _arhoEA_name = {FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
    _alpha_name = {FlowModelTwoPhase::VOLUME_FRACTION_LIQUID,
                   FlowModelTwoPhase::VOLUME_FRACTION_VAPOR};
    _velocity_name = {FlowModelTwoPhase::VELOCITY_LIQUID, FlowModelTwoPhase::VELOCITY_VAPOR};
    _enthalpy_name = {FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_LIQUID,
                      FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_VAPOR};
    _pressure_name = {FlowModelTwoPhase::PRESSURE_LIQUID, FlowModelTwoPhase::PRESSURE_VAPOR};
  }
  else
    logModelNotImplementedError(_flow_model_id);
}

void
FreeBoundary::check() const
{
  FlowBoundary::check();

  if ((_spatial_discretization == FlowModel::rDG) && (_flow_model_id == THM::FM_TWO_PHASE_NCG))
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
FreeBoundary::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    addMooseObjectsBoundaryFlux3Eqn();
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    if (_spatial_discretization == FlowModel::CG)
      addMooseObjectsCG();
    else if (_spatial_discretization == FlowModel::rDG)
      addMooseObjectsBoundaryFlux7Eqn();
  }
}

void
FreeBoundary::addMooseObjectsCG()
{
  // NOTE: Currently no integration by parts is performed on the volume fraction
  // equation, so there are no boundary terms; thus, nothing needs to be done
  // here. However, if the volume fraction equation is ever modified and uses
  // integration by parts, then the resulting BC will need to be added here.

  std::vector<VariableName> cv_area{FlowModel::AREA};
  std::vector<VariableName> cv_beta{FlowModelTwoPhase::BETA};

  const unsigned int n_phases = _is_two_phase ? 2 : 1;
  for (unsigned int k = 0; k < n_phases; k++)
  {
    {
      std::string class_name = "OneDMassFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _arhoA_name[k];
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<std::vector<VariableName>>("arhouA") = {_arhouA_name[k]};
      _sim.addBoundaryCondition(class_name, genName(name(), "mass_bc" + _phase_suffix[k]), params);
    }
    {
      std::string class_name = "OneDMomentumFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _arhouA_name[k];
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<std::vector<VariableName>>("arhoA") = {_arhoA_name[k]};
      params.set<std::vector<VariableName>>("arhouA") = {_arhouA_name[k]};
      params.set<std::vector<VariableName>>("arhoEA") = {_arhoEA_name[k]};
      params.set<std::vector<VariableName>>("vel") = {_velocity_name[k]};
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<MaterialPropertyName>("p") = _pressure_name[k];
      if (_is_two_phase)
      {
        params.set<bool>("is_liquid") = _is_liquid[k];
        params.set<std::vector<VariableName>>("beta") = cv_beta;
        params.set<std::vector<VariableName>>("alpha") = {_alpha_name[k]};
      }
      _sim.addBoundaryCondition(class_name, genName(name(), "mom_bc" + _phase_suffix[k]), params);
    }
    {
      std::string class_name = "OneDEnergyFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _arhoEA_name[k];
      params.set<Real>("normal") = _normal;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<std::vector<VariableName>>("arhoA") = {_arhoA_name[k]};
      params.set<std::vector<VariableName>>("arhouA") = {_arhouA_name[k]};
      params.set<std::vector<VariableName>>("arhoEA") = {_arhoEA_name[k]};
      params.set<std::vector<VariableName>>("vel") = {_velocity_name[k]};
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("H") = {_enthalpy_name[k]};
      params.set<MaterialPropertyName>("p") = _pressure_name[k];
      if (_is_two_phase)
      {
        params.set<bool>("is_liquid") = _is_liquid[k];
        params.set<std::vector<VariableName>>("beta") = cv_beta;
        params.set<std::vector<VariableName>>("alpha") = {_alpha_name[k]};
      }
      _sim.addBoundaryCondition(class_name, genName(name(), "erg_bc" + _phase_suffix[k]), params);
    }
  }
}

void
FreeBoundary::addMooseObjectsBoundaryFlux3Eqn()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux3EqnFreeOutflow";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
FreeBoundary::addMooseObjectsBoundaryFlux7Eqn()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux7EqnGhostFreeOutflow";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
  }

  // BCs
  addWeakBC7Eqn(boundary_flux_name);
}
