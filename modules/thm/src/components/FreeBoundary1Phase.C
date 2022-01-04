#include "FreeBoundary1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", FreeBoundary1Phase);

InputParameters
FreeBoundary1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  return params;
}

FreeBoundary1Phase::FreeBoundary1Phase(const InputParameters & parameters)
  : FlowBoundary1Phase(parameters)
{
}

void
FreeBoundary1Phase::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnFreeOutflow";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, _boundary_uo_name, params);
  }

  // BCs
  addWeakBC3Eqn();
}
