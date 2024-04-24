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
class AddFESpaceAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFESpaceAction(const InputParameters & parameters);

  virtual void act() override;
};
