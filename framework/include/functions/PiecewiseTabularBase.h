//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseBase.h"
#include "PiecewiseTabularInterface.h"
#include "LinearInterpolation.h"

/**
 * Function base which provides a piecewise approximation to a provided (x,y) point data set via
 * input parameter specifications. Derived classes, which control the order (constant, linear) of
 * the approximation and how the (x,y) data set is generated, should be used directly.
 */
class PiecewiseTabularBase : public PiecewiseBase, public PiecewiseTabularInterface
{
public:
  static InputParameters validParams();

  PiecewiseTabularBase(const InputParameters & parameters);

  /// Needed to load data from user objects that are not available at construction
  void initialSetup() override;

protected:
  /// function value scale factor
  const Real & _scale_factor;

private:
  using PiecewiseTabularInterface::buildFromFile;
  using PiecewiseTabularInterface::buildFromJSON;
  using PiecewiseTabularInterface::buildFromXandY;
  using PiecewiseTabularInterface::buildFromXY;
};
