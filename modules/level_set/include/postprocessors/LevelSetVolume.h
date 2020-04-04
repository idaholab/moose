//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementVariablePostprocessor.h"

/**
 * Postprocessor to compute the area/volume inside and outside of a level set contour.
 */
class LevelSetVolume : public ElementVariablePostprocessor
{
public:
  static InputParameters validParams();

  LevelSetVolume(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void computeQpValue() override{};

protected:
  /// The accumulated volume to return as a PostprocessorValue
  Real _volume;

  /// The level set contour to consider for computing inside vs. outside of the volume
  const Real & _threshold;

  /// Flag for triggering the internal volume calculation
  const bool _inside;
};
