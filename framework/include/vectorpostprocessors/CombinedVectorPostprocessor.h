//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 * CombinedVectorPostprocessor is a type of VectorPostprocessor that outputs the
 * values of multiple vectors from other vectorpostprocessors.
 */
class CombinedVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  CombinedVectorPostprocessor(const InputParameters & parameters);

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize() override;

  /**
   * Populates the postprocessor vector of values with the supplied vectorpostprocessors
   */
  virtual void execute() override;

protected:
  /// The VectorPostprocessorValue object where the results are stored
  std::vector<VectorPostprocessorValue *> _vpp_vecs;

  /// The vector of VectorPostprocessorValue objects that are used to get the values of the vectorpostprocessors
  std::vector<const VectorPostprocessorValue *> _vectorpostprocessor_values;

  /// A filler value to place on vectors that are smaller than the longest vector
  const Real _filler_value;
};
