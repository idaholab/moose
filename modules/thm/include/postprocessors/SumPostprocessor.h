#pragma once

#include "GeneralPostprocessor.h"

class SumPostprocessor;

template <>
InputParameters validParams<SumPostprocessor>();

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
  Real _sum;

  const PostprocessorValue & _a;
  const PostprocessorValue & _b;
};
