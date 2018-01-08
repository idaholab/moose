/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetProblem.h"
#include "LevelSetTypes.h"

#include "MultiAppTransfer.h"

template <>
InputParameters
validParams<LevelSetProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addClassDescription("A specilized problem class that adds a custom call to "
                             "MultiAppTransfer execution to transfer adaptivity for the level set "
                             "reinitialization.");
  return params;
}

LevelSetProblem::LevelSetProblem(const InputParameters & parameters) : FEProblem(parameters) {}

bool
LevelSetProblem::adaptMesh()
{
  // reset cycle counter
  _cycles_completed = 0;

  if (!_adaptivity.isAdaptivityDue())
    return false;

  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();

  Moose::perf_log.push("Adaptivity: adaptMesh()", "Execution");

  bool mesh_changed = false;

  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {
    _console << "Adaptivity step " << i + 1 << " of " << cycles_per_step << '\n';

    // Markers were already computed once by Executioner
    if (_adaptivity.getRecomputeMarkersFlag() && i > 0)
      computeMarkers();

    execMultiAppTransfers(LevelSet::EXEC_ADAPT_MESH, MultiAppTransfer::TO_MULTIAPP);

    if (_adaptivity.adaptMesh())
    {
      mesh_changed = true;

      meshChangedHelper(true); // This may be an intermediate change
      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      break;
    }

    // Show adaptivity progress
    _console << std::flush;
  }

  // We're done with all intermediate changes; now get systems ready
  // for real if necessary.
  if (mesh_changed)
    _eq.reinit_systems();

  Moose::perf_log.pop("Adaptivity: adaptMesh()", "Execution");

  return mesh_changed;
}
