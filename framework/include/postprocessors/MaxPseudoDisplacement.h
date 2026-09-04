//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Reports max over nodes of ||d(n)||, the largest magnitude of the pseudo-displacement d
 * accumulated by the [Remeshing] engine since the last remesh event.
 */
class MaxPseudoDisplacement : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MaxPseudoDisplacement(const InputParameters & parameters);

  virtual void initialize() override;

  /// Take the maximum magnitude over the local and ghosted nodes this rank holds
  virtual void execute() override;

  /// Reduce the local maxima into the parallel consistent answer
  virtual void finalize() override;

  /// @return max over nodes of ||d(n)||, or zero when the engine does not move the mesh
  virtual Real getValue() const override;

private:
  /// max over nodes of ||d(n)||, local to this rank until finalize() reduces it
  Real _max_pseudo_displacement;
};
