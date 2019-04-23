//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FACEFACECONSTRAINT_H
#define FACEFACECONSTRAINT_H

#include "MortarConstraint.h"

class FaceFaceConstraint;

template <>
InputParameters validParams<FaceFaceConstraint>();

/**
 * This is a deprecated object!  Use MortarConstraint instead!
 */
class FaceFaceConstraint : public MortarConstraint
{
public:
  FaceFaceConstraint(const InputParameters & params) : MortarConstraint(params)
  {
    mooseDeprecated("FaceFaceConstraint is deprecated!  Use MortarConstraint instead!");
  }
};

#endif
