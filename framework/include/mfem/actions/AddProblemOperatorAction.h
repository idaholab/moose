#ifdef MFEM_ENABLED

#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action controlling the construction of the
 * required MFEM ProblemOperator.
 */
class AddProblemOperatorAction : public Action
{
public:
  static InputParameters validParams();

  AddProblemOperatorAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
