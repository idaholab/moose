//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVBoundaryCondition.h"

/**
 * Base class for finite volume Dirichlet boundaray conditions. A base class is
 * needed because Dirichlet BCs must be handled specially when computing fluxes
 * on faces in the FV method.
 */
class FVDirichletBCBase : public FVBoundaryCondition
{
public:
  FVDirichletBCBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual ADReal boundaryValue(const FaceInfo & fi) const = 0;
};
