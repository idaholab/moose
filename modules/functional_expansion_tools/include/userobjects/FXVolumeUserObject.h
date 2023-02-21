//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariableUserObject.h"
#include "FXIntegralBaseUserObject.h"

/**
 * This volumetric FX calculates the value
 */
class FXVolumeUserObject final : public FXIntegralBaseUserObject<ElementIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  FXVolumeUserObject(const InputParameters & parameters);

protected:
  // Overrides from FXIntegralBaseUserObject
  virtual Point getCentroid() const;
  virtual Real getVolume() const;
};
