//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "PointSamplerBase.h"

/**
 * Base class for sampling variable(s) at points
 */
class PointVariableSamplerBase : public PointSamplerBase,
                                 public CoupleableMooseVariableDependencyIntermediateInterface,
                                 public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  PointVariableSamplerBase(const InputParameters & parameters);

  virtual ~PointVariableSamplerBase() {}

  virtual void initialize();
  virtual void execute();

  void setPointsVector(const std::vector<Point> & points);
  void transferPointsVector(std::vector<Point> && points);

protected:
  /// Current quadrature point
  unsigned int _qp;
};
