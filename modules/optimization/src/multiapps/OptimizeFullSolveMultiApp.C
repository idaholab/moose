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

  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;

  return params;
}

OptimizeFullSolveMultiApp::OptimizeFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters)
{
  // initilialize mpi for a single subapp per multiapp (this is the default that would usually
  // happen if use_positions = true but that doesn't make any sense for what I'm doing)
  init(1);
}
