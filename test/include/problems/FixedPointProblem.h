//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

class FixedPointProblem;

template <>
InputParameters validParams<FixedPointProblem>();

/**
 * FEProblem derived class for lagging a tagged residual.
 */
class FixedPointProblem : public FEProblem
{
public:
  FixedPointProblem(const InputParameters & params);

  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual) override;
  virtual void computeFullResidual(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual);

protected:
  TagName _tag_previous;
  TagID _tag_id;
  NumericVector<Number> & _residual_previous;
};

