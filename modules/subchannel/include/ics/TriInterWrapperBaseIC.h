//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "TriSubChannelBaseIC.h"

class TriInterWrapperMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class TriInterWrapperBaseIC : public InitialCondition
{
public:
  TriInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is TriInterWrapperMesh and if not, report an error.
   */
  TriInterWrapperMesh & getMesh(MooseMesh & mesh);

  TriInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
