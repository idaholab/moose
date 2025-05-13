//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "BlockRestrictable.h"

/**
 * Positions from the extrema of a functor
 */
class FunctorExtremaPositions : public Positions,
                                public NonADFunctorInterface,
                                public BlockRestrictable
{
public:
  static InputParameters validParams();
  FunctorExtremaPositions(const InputParameters & parameters);
  virtual ~FunctorExtremaPositions() = default;

  void initialize() override;

private:
  /// Functor providing the value
  const Moose::Functor<Real> & _functor;

  /// Number of extrema to keep track of
  const unsigned int _n_extrema;

  /// Type of extreme value we are going to compute
  enum class ExtremeType
  {
    MAX,
    MIN,
    MAX_ABS
  } _type;

  /// Values of the functor at the extrema
  std::vector<Real> _positions_values;
};
