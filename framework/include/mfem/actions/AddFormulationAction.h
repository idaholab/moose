#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to have a section of the input file like the
 * following to specify the EM formulation to build.
 *
 * [Formulation]
 * []
 */
class AddFormulationAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFormulationAction(const InputParameters & parameters);

  virtual void act() override;
};
