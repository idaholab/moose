#ifdef MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"

/**
 * This class allows us to have a section of the input file like the following
 * specifying the solver to use and the solve options.
 *
 * [Preconditioner]
 * []
 */
class AddMFEMPreconditionerAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMPreconditionerAction(const InputParameters & parameters);

  void act() override;
};

#endif
