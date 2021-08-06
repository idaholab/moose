//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVSymmetryVelocityBC.h"

/**
 * A symmetry boundary condition for the superficial velocity. It should be
 * used in conjunction with an INSFVSymmetryPressureBC.
 */
class PINSFVSymmetryVelocityBC : public INSFVSymmetryVelocityBC
{
public:
  static InputParameters validParams();
  PINSFVSymmetryVelocityBC(const InputParameters & params);
};
