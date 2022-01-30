#pragma once

#include "LayeredAverage.h"
#include "RZSymmetry.h"

/**
 * The same functionality as LayeredAverage but for arbitrary RZ symmetry
 *
 * NOTE: this is a temporary object until MOOSE can do this
 */
class LayeredAverageRZ : public LayeredAverage, public RZSymmetry
{
public:
  LayeredAverageRZ(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual Real computeIntegral() override;

public:
  static InputParameters validParams();
};
