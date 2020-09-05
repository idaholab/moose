//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class VectorPostprocessorDifferenceMeasure;

template <>
InputParameters validParams<VectorPostprocessorDifferenceMeasure>();

/**
 * Compares two vector post-processors of equal size and computes a measure of their difference
 *
 * This post-processor implements two measure of the distance between
 * vector post-processor \c a and vector post-processor \c b:
 * \li <b>difference</b>: <tt>Summed difference between a and b</tt>?
 * \li <b>L2</b>: <tt>L2 or Euclidean distance between a and b</tt>?
 */
class VectorPostprocessorDifferenceMeasure : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  VectorPostprocessorDifferenceMeasure(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// Values of the first vector post-processor to compare
  const VectorPostprocessorValue & _values_a;
  /// Values of the second vector post-processor to compare
  const VectorPostprocessorValue & _values_b;
  /// Comparison type
  enum class DifferenceType
  {
    L2,
    DIFFERENCE
  };
  /// Type of Difference to perform
  const DifferenceType _difference_type;
  /// The difference value
  PostprocessorValue _difference_value;

  /// Compute summed difference between two vectors
  void computeDifference();
  /// Compute L2 or Euclidean distance between two vectors
  void computeL2();
  /// simple error checking on vectors being compared
  void errorCheck();
};
