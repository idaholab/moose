//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "ResidualObject.h"
#include "ParallelUniqueId.h"
#include "DistributionInterface.h"
#include "GeometricSearchInterface.h"
#include "BoundaryRestrictableRequired.h"

/**
 * Base class for creating new types of boundary conditions.
 */
class BoundaryCondition : public ResidualObject,
                          public BoundaryRestrictableRequired,
                          public DistributionInterface,
                          public GeometricSearchInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   * @param nodal Whether this BC is applied to nodes or not
   */
  BoundaryCondition(const InputParameters & parameters, bool nodal);

  static InputParameters validParams();

  /**
   * Hook for turning the boundary condition on and off.
   *
   * It is not safe to use variable values in this function, since (a) this is not called inside a
   * quadrature loop,
   * (b) reinit() is not called, thus the variables values are not computed.
   * NOTE: In NodalBC-derived classes, we can use the variable values, since renitNodeFace() was
   * called before calling
   * this method. However, one has to index into the values manually, i.e. not using _qp.
   * @return true if the boundary condition should be applied, otherwise false
   */
  virtual bool shouldApply() { return true; }
};
