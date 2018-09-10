#include "SolidWall.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Pipe.h"
#include "Factory.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("RELAP7App", SolidWall);

template <>
InputParameters
validParams<SolidWall>()
{
  InputParameters params = validParams<FlowBoundary>();
  return params;
}

SolidWall::SolidWall(const InputParameters & params) : FlowBoundary(params) {}

void
SolidWall::check() const
{
  if ((_spatial_discretization == FlowModel::rDG) && (_flow_model_id == RELAP7::FM_TWO_PHASE_NCG))
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
SolidWall::addMooseObjects1Phase()
{
  if (_spatial_discretization == FlowModel::CG)
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName(name(), "rhou"), params);
  }
  else if (_spatial_discretization == FlowModel::rDG)
  {
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_LINEAR, EXEC_NONLINEAR};

    // boundary flux user object
    const std::string boundary_flux_name = genName(name(), "boundary_flux");
    {
      const std::string class_name = "BoundaryFlux3EqnGhostWall";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim.addUserObject(class_name, boundary_flux_name, params);
    }

    // BCs
    addWeakBC3Eqn(boundary_flux_name);
  }
}

void
SolidWall::addMooseObjects2Phase()
{
  if (_spatial_discretization == FlowModel::CG)
  {
    {
      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("value") = 0.;
      _sim.addBoundaryCondition("DirichletBC", genName(name(), "arhouA_liquid"), params);
    }
    {
      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("value") = 0.;
      _sim.addBoundaryCondition("DirichletBC", genName(name(), "arhouA_vapor"), params);
    }
  }
  else if (_spatial_discretization == FlowModel::rDG)
  {
    ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
    userobject_execute_on = {EXEC_LINEAR, EXEC_NONLINEAR};

    // boundary flux user object
    const std::string boundary_flux_name = genName(name(), "boundary_flux");
    {
      const std::string class_name = "BoundaryFlux7EqnGhostWall";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim.addUserObject(class_name, boundary_flux_name, params);
    }

    // BCs
    addWeakBC7Eqn(boundary_flux_name);
  }
}

void
SolidWall::addMooseObjects()
{
  if (_flow_model_id == RELAP7::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_flow_model_id == RELAP7::FM_TWO_PHASE || _flow_model_id == RELAP7::FM_TWO_PHASE_NCG)
    addMooseObjects2Phase();
}
