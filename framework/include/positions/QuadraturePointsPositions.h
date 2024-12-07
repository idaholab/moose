//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"
#include "BlockRestrictable.h"

class MooseMesh;

/**
 * Positions of all the quadrature points
 */
class QuadraturePointsPositions : public Positions, BlockRestrictable
{
public:
  static InputParameters validParams();
  QuadraturePointsPositions(const InputParameters & parameters);

  virtual void initialize() override;

  /// Reference to the mesh
  MooseMesh & _mesh;

  /// Type of the quadrature
  libMesh::QuadratureType _q_type;
  /// Order of the quadrature
  libMesh::Order _q_order;
};
