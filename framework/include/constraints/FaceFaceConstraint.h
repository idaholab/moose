//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

/**
 * This is a deprecated object!  Use MortarConstraint instead!
 */
class FaceFaceConstraint : public ADMortarConstraint
{
public:
  FaceFaceConstraint(const InputParameters & params) : ADMortarConstraint(params)
  {
    mooseDeprecated("FaceFaceConstraint is deprecated!  Use MortarConstraint instead!");
  }
};
