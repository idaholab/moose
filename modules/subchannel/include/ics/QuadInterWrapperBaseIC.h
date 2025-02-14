//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "InitialCondition.h"

class QuadInterWrapperMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadInterWrapperBaseIC : public InitialCondition
{
public:
  QuadInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  const QuadInterWrapperMesh & getMesh(const MooseMesh & mesh);

  const QuadInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
