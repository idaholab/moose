#include "SolidWall1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", SolidWall1Phase);

InputParameters
SolidWall1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  return params;
}

SolidWall1Phase::SolidWall1Phase(const InputParameters & params) : FlowBoundary1Phase(params) {}

void
SolidWall1Phase::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnGhostWall";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<Real>("normal") = _normal;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, _boundary_uo_name, params);
  }

  // BCs
  addWeakBC3Eqn();
}
