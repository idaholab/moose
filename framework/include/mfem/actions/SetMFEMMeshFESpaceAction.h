#ifdef MFEM_ENABLED

#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action ensuring the mesh uses the same FE space as
 * the displacement for mesh updates.
 */
class SetMFEMMeshFESpaceAction : public Action
{
public:
  static InputParameters validParams();

  SetMFEMMeshFESpaceAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
