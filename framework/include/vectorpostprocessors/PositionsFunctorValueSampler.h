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
#include "NonADFunctorInterface.h"

/**
 * Samples one or more functor(s) at points given by a Positions object
 */
class PositionsFunctorValueSampler : public PointSamplerBase, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  PositionsFunctorValueSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  /// Functors to sample
  std::vector<const Moose::Functor<Real> *> _functors;

  /// Positions to sample the functors at
  const Positions & _positions;
};
