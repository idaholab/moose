//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class WeightedGapUserObject;

/**
 * Total normal contact force that one mortar contact pair transmits to its secondary surface,
 * computed from the pair's weighted gap user object
 */
class MortarContactForce : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MortarContactForce(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override;

protected:
  /// The weighted gap user object of the contact pair
  const WeightedGapUserObject & _weighted_gap_uo;

  /// The force component to report (0, 1, 2), or 3 for the magnitude
  const unsigned int _component;

  /// The total contact force vector
  RealVectorValue _force;
};
