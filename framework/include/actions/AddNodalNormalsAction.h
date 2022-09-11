//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action to setup computation of nodal normals.
 *
 * The machinery behind the normal computation is the following:
 * - NodalNormalsPreprocessor is ran over the elements and gather the \f$ \int \d grad_phi \over \d
 * {x|y|z} \d Omega \f$ into three separate vectors
 *   (that live on AuxiliarySystem) - each for one component of the normal.
 * - NodalNormalsEvaluator is than ran over the boundary nodes and takes the above computed
 * integrals and normalizes it.
 *
 * NOTE: the auxiliary system has to have at least one variable on it, so that the vectors for nx,
 * ny and nz have non-zero length.
 */
class AddNodalNormalsAction : public Action
{
public:
  static InputParameters validParams();

  AddNodalNormalsAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// The supplied boundary name from the user
  std::vector<BoundaryName> _boundary;

  /// Flag for testing the existance of the corner boundary input
  bool _has_corners;

  /// The supplied boundary name for the corner boundary
  BoundaryName _corner_boundary;
};
