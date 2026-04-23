//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMultiAppTransfer.h"
#include "MFEMProblem.h"
#include "DisplacedProblem.h"

InputParameters
MFEMMultiAppTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<VariableName>>("variables",
                                                     "Variable(s) to store transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variables",
                                                     "Variable(s) to transfer from.");
  return params;
}

MFEMMultiAppTransfer::MFEMMultiAppTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _from_var_names(getParam<std::vector<VariableName>>("source_variables")),
    _to_var_names(getParam<std::vector<VariableName>>("variables"))
{
  if (numToVar() != numFromVar())
    paramError("source_variables", "Number of variables transferred must be same in both systems.");
}

void
MFEMMultiAppTransfer::execute()
{
  TIME_SECTION(
      "MFEMMultiAppTransfer::execute", 5, "Perform transfer to and/or from an MFEMProblem.");
  switch (_current_direction)
  {
    case TO_MULTIAPP:
      for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
        if (getToMultiApp()->hasLocalApp(i))
        {
          setActiveToProblem(getToMultiApp()->appProblemBase(i), i);
          setActiveFromProblem(getToMultiApp()->problemBase(), 0);
          transferVariables(true);
        }
      break;
    case FROM_MULTIAPP:
      for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
        if (getFromMultiApp()->hasLocalApp(i))
        {
          setActiveToProblem(getFromMultiApp()->problemBase(), 0);
          setActiveFromProblem(getFromMultiApp()->appProblemBase(i), i);
          transferVariables(true);
        }
      break;
    case BETWEEN_MULTIAPP:
      int transfers_done = 0;
      for (const auto from_app_id : make_range(getFromMultiApp()->numGlobalApps()))
        for (const auto to_app_id : make_range(getToMultiApp()->numGlobalApps()))
          if (getFromMultiApp()->hasLocalApp(from_app_id))
          {
            setActiveFromProblem(getFromMultiApp()->appProblemBase(from_app_id), from_app_id);
            if (getToMultiApp()->hasLocalApp(to_app_id))
            {
              setActiveToProblem(getToMultiApp()->appProblemBase(to_app_id), to_app_id);
              transferVariables(true);
              ++transfers_done;
            }
            else
            {
              transferVariables(false);
            }
          }
      if (!transfers_done)
        mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
                   "per multiapp involved on each rank");
      break;
  }
}

EquationSystems &
MFEMMultiAppTransfer::getlibMeshEquationSystem(FEProblemBase & problem, bool use_displaced) const
{
  if (use_displaced)
  {
    if (!problem.getDisplacedProblem())
      mooseError("No displaced problem to provide a displaced equation system");
    return problem.getDisplacedProblem()->es();
  }
  else
    return problem.es();
}

#endif
