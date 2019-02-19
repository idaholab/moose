#include "PeriodicJunction.h"
#include "NonlinearSystemBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "libmesh/periodic_boundary.h"
#include "THMMesh.h"

registerMooseObject("THMApp", PeriodicJunction);

template <>
InputParameters
validParams<PeriodicJunction>()
{
  InputParameters params = validParams<FlowJunction>();
  return params;
}

PeriodicJunction::PeriodicJunction(const InputParameters & params) : FlowJunction(params) {}

void
PeriodicJunction::setupMesh()
{
  FlowJunction::setupMesh();

  _translation_vector = _mesh.nodeRef(_nodes[0]) - _mesh.nodeRef(_nodes[1]);
}

void
PeriodicJunction::check() const
{
  FlowJunction::check();

  checkNumberOfConnections(2);

  if (!(_flow_model_id == THM::FM_SINGLE_PHASE || _flow_model_id == THM::FM_TWO_PHASE))
    logModelNotImplementedError(_flow_model_id);
}

void
PeriodicJunction::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    const std::vector<VariableName> variables{
        FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

    addPeriodicBC(variables);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    std::vector<VariableName> variables{
        FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
        FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
        FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
        FlowModelTwoPhase::ALPHA_RHO_A_VAPOR,
        FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR,
        FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR,
    };

    if (_phase_interaction)
      variables.push_back(FlowModelTwoPhase::BETA);

    addPeriodicBC(variables);
  }
}

void
PeriodicJunction::addPeriodicBC(const std::vector<VariableName> variables) const
{
  FEProblem & fe_problem = _sim.feproblem();
  NonlinearSystemBase & nl_system = fe_problem.getNonlinearSystemBase();

  PeriodicBoundary periodic_boundary(_translation_vector);
  periodic_boundary.myboundary = _boundary_ids[1];
  periodic_boundary.pairedboundary = _boundary_ids[0];

  for (const auto & variable : variables)
  {
    const unsigned int var_num = nl_system.getVariable(0, variable).number();
    periodic_boundary.set_variable(var_num);
    _mesh.addPeriodicVariable(
        var_num, periodic_boundary.myboundary, periodic_boundary.pairedboundary);
  }

  fe_problem.addGhostedBoundary(periodic_boundary.myboundary);
  fe_problem.addGhostedBoundary(periodic_boundary.pairedboundary);

  nl_system.dofMap().add_periodic_boundary(periodic_boundary);
}
