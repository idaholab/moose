#include "SolidWall.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "Pipe.h"
#include "Factory.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

template <>
InputParameters
validParams<SolidWall>()
{
  InputParameters params = validParams<FlowBoundary>();
  return params;
}

SolidWall::SolidWall(const InputParameters & params) : FlowBoundary(params) {}

void
SolidWall::check()
{
  if ((_spatial_discretization == FlowModel::rDG) && (_flow_model_id == RELAP7::FM_TWO_PHASE))
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
      const std::string class_name = "Euler1DVarAreaWallBoundaryFlux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("rdg_flux") = _rdg_flux_name;
      params.set<bool>("implicit") = _implicit_rdg;
      params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
      _sim.addUserObject(class_name, boundary_flux_name, params);
    }

    // BCs
    {
      const std::string class_name = "Euler1DVarAreaBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<UserObjectName>("boundary_flux") = boundary_flux_name;
      params.set<std::vector<VariableName>>("A") = {FlowModelSinglePhase::AREA};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = _implicit_rdg;

      // mass
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
      _sim.addBoundaryCondition(
          class_name, genName(name(), class_name, FlowModelSinglePhase::RHOA), params);

      // momentum
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      _sim.addBoundaryCondition(
          class_name, genName(name(), class_name, FlowModelSinglePhase::RHOUA), params);

      // energy
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      _sim.addBoundaryCondition(
          class_name, genName(name(), class_name, FlowModelSinglePhase::RHOEA), params);
    }
  }
}

void
SolidWall::addMooseObjects2Phase()
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

void
SolidWall::addMooseObjects()
{
  if (_flow_model_id == RELAP7::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_flow_model_id == RELAP7::FM_TWO_PHASE)
    addMooseObjects2Phase();
}
