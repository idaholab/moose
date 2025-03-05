#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action ensuring the mesh uses the same FE space as
 * the displacement for mesh updates.
 */
class SetMeshFESpaceAction : public Action
{
public:
  static InputParameters validParams();

  SetMeshFESpaceAction(const InputParameters & parameters);

  virtual void act() override;
};
