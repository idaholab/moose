//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"

/**
 * Combines multiple marker fields.  The most conservative (requesting additional refinement) wins.
 */
class ComboMarker : public Marker
{
public:
  static InputParameters validParams();

  ComboMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /// Names of the markers contributing to the combo
  const std::vector<MarkerName> _names;

  /// Pointers to the markers contributing to the Combo marker
  std::vector<const VariableValue *> _markers;

  /// Boolean to keep track of whether any marker does not have the same block restriction
  bool _block_restriction_mismatch;

  /// Pointers to the variables for the markers
  std::vector<const MooseVariableFieldBase *> _marker_variables;
};
