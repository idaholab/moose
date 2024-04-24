#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to have a section of the input file like the
 * following to add MFEM coefficients to the problem.
 *
 * [Coefficients]
 * []
 */
class AddCoefficientAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddCoefficientAction(const InputParameters & parameters);

  virtual void act() override;
};
