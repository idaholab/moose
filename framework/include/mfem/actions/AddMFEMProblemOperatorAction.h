#ifdef MFEM_ENABLED

#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action controlling the construction of the
 * required MFEM ProblemOperator.
 */
class AddMFEMProblemOperatorAction : public Action
{
public:
  static InputParameters validParams();

  AddMFEMProblemOperatorAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
