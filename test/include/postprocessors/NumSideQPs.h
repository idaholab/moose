//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideIntegralPostprocessor.h"

/**
 * An object for testing that the specified quadrature order is used.  It
 * counts the number of quadrature points.
 */
class NumSideQPs : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  NumSideQPs(const InputParameters & parameters);
  virtual ~NumSideQPs();
  virtual Real computeIntegral();
  virtual Real computeQpIntegral();
};
