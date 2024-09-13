#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action controlling the construction of the
 * required MFEM ProblemBuilder.
 */
class AddProblemBuilderAction : public Action
{
public:
  static InputParameters validParams();

  AddProblemBuilderAction(const InputParameters & parameters);

  virtual void act() override;
};
