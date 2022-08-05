#pragma once

#include "AuxKernel.h"
#include "RadialAverage.h"
/**
 * Auxkernel to output the averaged material value from RadialAverage
 */
class RadialAverageAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RadialAverageAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  const RadialAverage::Result & _average;
  RadialAverage::Result::const_iterator _elem_avg;
};
