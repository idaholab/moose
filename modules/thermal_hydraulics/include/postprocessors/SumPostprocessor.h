#pragma once

#include "GeneralPostprocessor.h"

/**
 * Computes a sum of postprocessor values
 *
 * TOOD: Generalize to take a vector of PPS names and possibly mode to MOOSE
 */
class SumPostprocessor : public GeneralPostprocessor
{
public:
  SumPostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();

protected:
  /// Postprocessors to add up
  std::vector<const PostprocessorValue *> _values;

public:
  static InputParameters validParams();
};
