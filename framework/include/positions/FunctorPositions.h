//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"
#include "FunctorInterface.h"

/**
 * Positions from groups of three functors
 */
class FunctorPositions : public Positions, public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  FunctorPositions(const InputParameters & parameters);
  virtual ~FunctorPositions() = default;

  void initialize() override;

private:
  /// Vector of pointers to the functors for each coordinate (inner-ordering is coordinates)
  std::vector<const Moose::Functor<Real> *> _pos_functors;
};
