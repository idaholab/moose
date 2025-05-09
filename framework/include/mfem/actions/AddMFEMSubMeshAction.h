#ifdef MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to have a section of the input file like the
 * following to add MFEM submeshes to the problem.
 *
 * [SubMeshes]
 * []
 */
class AddMFEMSubMeshAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMSubMeshAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
