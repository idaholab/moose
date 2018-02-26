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
  InputParameters params = validParams<PipeBoundary>();
  return params;
}

SolidWall::SolidWall(const InputParameters & params) : PipeBoundary(params) {}

void
SolidWall::addMooseObjects1Phase()
{
  {
    InputParameters params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("value") = 0.;
    _sim.addBoundaryCondition("DirichletBC", genName(name(), "rhou"), params);
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
