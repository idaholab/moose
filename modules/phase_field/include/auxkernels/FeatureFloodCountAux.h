//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "FeatureFloodCount.h"

// Forward Declarations
class GrainTrackerInterface;

/**
 * Function auxiliary value
 */
class FeatureFloodCountAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  FeatureFloodCountAux(const InputParameters & parameters);

  virtual ~FeatureFloodCountAux() {}

protected:
  using FieldType = FeatureFloodCount::FieldType;

  virtual Real computeValue() override;
  virtual void precalculateValue() override;

  /// Function being used to compute the value of this kernel
  const FeatureFloodCount & _flood_counter;

  const std::size_t _var_idx;
  const FieldType _field_type;
  bool _var_coloring;

  /// precalculated element value
  Real _value;
};
