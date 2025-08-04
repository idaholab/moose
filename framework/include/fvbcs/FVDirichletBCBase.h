//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  virtual ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const = 0;

protected:
  /// A pair keeping track of the variable and system numbers
  const std::pair<unsigned int, unsigned int> _var_sys_numbers_pair;
};
