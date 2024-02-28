//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EulerAngles.h"
#include "GeneralUserObject.h"

/**
 * Abstract base class for user objects that implement the Euler Angle provider
 * interface.
 */
class EulerAngleProvider : public GeneralUserObject
{
public:
  static InputParameters validParams();

  EulerAngleProvider(const InputParameters & parameters) : GeneralUserObject(parameters) {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const = 0;
  virtual unsigned int getGrainNum() const = 0;
};
