//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class InterfaceMeshCutUserObjectBase;

/**
 * Calculate level set values for an interface that is defined by a lower-dimensional mesh
 */
class MeshCutLevelSetAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MeshCutLevelSetAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Pointer to the InterfaceMeshCutUserObject object
  const InterfaceMeshCutUserObjectBase * _mesh_cut_uo;
};
