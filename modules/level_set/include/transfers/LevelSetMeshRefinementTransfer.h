/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETMESHREFINEMENTTRANSFER_H
#define LEVELSETMESHREFINEMENTTRANSFER_H

#include "MultiAppCopyTransfer.h"

// Forward declarations
class LevelSetMeshRefinementTransfer;

template<>
InputParameters validParams<LevelSetMeshRefinementTransfer>();

/**
 * Copies the refinement marker from the master to the sub-application.
 */
class LevelSetMeshRefinementTransfer : public MultiAppCopyTransfer
{
public:
  LevelSetMeshRefinementTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

};

#endif // LEVELSETMESHREFINEMENTTRANSFER_H
