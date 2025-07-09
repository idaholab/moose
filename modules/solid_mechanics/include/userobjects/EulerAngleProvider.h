//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  virtual const EulerAngles & getEulerAngles(unsigned int i) const
  {
    mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
    return _angles[i];
  };

  virtual unsigned int getGrainNum() const { return _angles.size(); };

protected:
  std::vector<EulerAngles> _angles;
};
