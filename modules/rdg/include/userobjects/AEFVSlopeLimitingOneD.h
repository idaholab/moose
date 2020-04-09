//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeLimitingBase.h"

// Forward Declarations

/**
 * One-dimensional slope limiting to get
 * the limited slope of cell average variable
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVSlopeLimitingOneD : public SlopeLimitingBase
{
public:
  static InputParameters validParams();

  AEFVSlopeLimitingOneD(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const override;

protected:
  /// the input variable
  MooseVariable * _u;

  /// One-D slope limiting scheme
  MooseEnum _scheme;
};
