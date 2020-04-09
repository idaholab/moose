//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariableUserObject.h"

#include "FXIntegralBaseUserObject.h"

/**
 * This class provides the base for generating a functional expansion on a boundary by inheriting
 * from FXIntegralBaseUserObject and providing SideIntegralVariableUserObject as the template
 * parameter
 */
class FXBoundaryBaseUserObject : public FXIntegralBaseUserObject<SideIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  FXBoundaryBaseUserObject(const InputParameters & parameters);

protected:
  // Overrides from FXIntegralBaseUserObject
  virtual Point getCentroid() const final;
  virtual Real getVolume() const final;
};
