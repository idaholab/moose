//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateTrainer.h"

class NearestPointTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();
  NearestPointTrainer(const InputParameters & parameters);
  virtual void preTrain() override;
  virtual void train() override;
  virtual void postTrain() override;

protected:
  /// Map containing sample points and the results
  std::vector<std::vector<Real>> & _sample_points;

  /// Response value
  const Real & _response;

  /// Predictor values
  const std::vector<const Real *> _predictors;
};
