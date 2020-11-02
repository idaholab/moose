#include "OptimizeFullSolveMultiApp.h"

// isopod
#include "IsopodAppTypes.h"

registerMooseObject("isopodApp", OptimizeFullSolveMultiApp);

InputParameters
OptimizeFullSolveMultiApp::validParams()
{
  InputParameters params = FullSolveMultiApp::validParams();
  params.addClassDescription("This is FullSolveMultiApp with some extra flags registered.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN);
  params.addParam<ExecFlagEnum>(
      "execute_on", exec_enum, "List of flags indicating when this multiapp should solve.");
  return params;
}

OptimizeFullSolveMultiApp::OptimizeFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters)
{
}
