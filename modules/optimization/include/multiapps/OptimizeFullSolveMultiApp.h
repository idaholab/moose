#pragma once
// fixme lynn this is included because MultiApp probably needs it
#include "libmesh/numeric_vector.h"
// MOOSE includes
#include "FullSolveMultiApp.h"

/**
 * This is FullSolveMultiApp with some extra flags registered.
 */
class OptimizeFullSolveMultiApp : public FullSolveMultiApp
{
public:
  static InputParameters validParams();
  OptimizeFullSolveMultiApp(const InputParameters & parameters);
};
