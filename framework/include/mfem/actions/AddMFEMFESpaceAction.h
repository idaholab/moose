#ifdef MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to have a section of the input file like the
 * following to add MFEM coefficients to the problem.
 *
 * [FESpaces]
 * []
 */
class AddMFEMFESpaceAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMFESpaceAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
