//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SMOOTHMESH_H
#define SMOOTHMESH_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class SmoothMesh;

template <>
InputParameters validParams<SmoothMesh>();

/**
 * MeshModifier for doing mesh smoothing
 */
class SmoothMesh : public MeshModifier
{
public:
  SmoothMesh(const InputParameters & parameters);

private:
  virtual void modify() override;

  /// The number of smoothing passes to do
  unsigned int _iterations;
};

#endif // SMOOTHMESH_H
