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
class AddVectorCoefficientAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddVectorCoefficientAction(const InputParameters & parameters);

  virtual void act() override;
};
