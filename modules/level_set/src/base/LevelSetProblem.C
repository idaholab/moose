/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetProblem.h"
#include "MultiAppTransfer.h"

template<>
InputParameters validParams<LevelSetProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addClassDescription("A specilized problem class that adds a custom call to MultiAppTransfer execution to transfer adaptivity for the level set reinitialization.");
  return params;

}

LevelSetProblem::LevelSetProblem(const InputParameters & parameters) :
    FEProblem(parameters)
{
}

void
LevelSetProblem::adaptMesh()
{
  if (!_adaptivity.isAdaptivityDue())
    return;

  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();
  _cycles_completed = 0;
  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {

    execMultiAppTransfers(EXEC_CUSTOM, MultiAppTransfer::TO_MULTIAPP);

    _console << "Adaptivity step " << i+1 << " of " << cycles_per_step << '\n';
    // Markers were already computed once by Executioner
    if (_adaptivity.getRecomputeMarkersFlag() && i > 0)
      computeMarkers();
    if (_adaptivity.adaptMesh())
    {
      meshChanged();
      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      return;
    }

    // Show adaptivity progress
    _console << std::flush;
  }
}
