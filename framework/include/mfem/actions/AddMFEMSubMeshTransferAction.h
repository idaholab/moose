#ifdef MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to add transfers between MFEM parent meshes 
 * and submeshes to the [Transfers] block.
 */
class AddMFEMSubMeshTransferAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMSubMeshTransferAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
