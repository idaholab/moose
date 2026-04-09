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
  params.addRequiredParam<std::vector<VariableName>>("variable",
                                                     "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  return params;
}

MFEMMultiAppTransfer::MFEMMultiAppTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<VariableName>>("variable"))
{
  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");
}

void
MFEMMultiAppTransfer::execute()
{
  TIME_SECTION(
      "MFEMMultiAppTransfer::execute", 5, "Perform transfer to and/or from an MFEMProblem.");
  switch (_current_direction)
  {
    case TO_MULTIAPP:
      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
        if (getToMultiApp()->hasLocalApp(i))
        {
          setActiveToProblem(getToMultiApp()->appProblemBase(i));
          setActiveFromProblem(getToMultiApp()->problemBase());
          transferVariables();
        }
      break;
    case FROM_MULTIAPP:
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
        if (getFromMultiApp()->hasLocalApp(i))
        {
          setActiveToProblem(getFromMultiApp()->problemBase());
          setActiveFromProblem(getFromMultiApp()->appProblemBase(i));
          transferVariables();
        }
      break;
    case BETWEEN_MULTIAPP:
      int transfers_done = 0;
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
        if (getFromMultiApp()->hasLocalApp(i) && getToMultiApp()->hasLocalApp(i))
        {
          setActiveToProblem(getToMultiApp()->appProblemBase(i));
          setActiveFromProblem(getFromMultiApp()->appProblemBase(i));
          transferVariables();
          ++transfers_done;
        }
      if (!transfers_done)
        mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
                   "per multiapp involved on each rank");
      break;
  }
}

void
MFEMMultiAppTransfer::checkSiblingsTransferSupported() const
{
  // Check that we are in the supported configuration: same number of source and target apps
  // The allocation of the child apps on the processors must be the same
  if (getFromMultiApp()->numGlobalApps() == getToMultiApp()->numGlobalApps())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) + getToMultiApp()->hasLocalApp(i) == 1)
        mooseError("Child application allocation on parallel processes must be the same to support "
                   "siblings variable field copy transfer");
  }
  else
    mooseError("Number of source and target child apps must match for siblings transfer");
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
