#pragma once

#include "MooseObjectAction.h"

/**
 * This class allows us to have a section of the input file like the following
 * specifying the solver to use and the solve options.
 *
 * [Solver]
 * []
 */
class AddMFEMSolverAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMSolverAction(const InputParameters & parameters);

  void act() override;
};
