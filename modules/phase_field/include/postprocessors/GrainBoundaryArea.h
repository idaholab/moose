/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINBOUNDARYAREA_H
#define GRAINBOUNDARYAREA_H

#include "ElementIntegralPostprocessor.h"

class GrainBoundaryArea;

template <>
InputParameters validParams<GrainBoundaryArea>();

/**
 * Calculate total grain boundary length in 2D and area in 3D.
 */
class GrainBoundaryArea : public ElementIntegralPostprocessor
{
public:
  GrainBoundaryArea(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  /// Number of order parameters
  const unsigned int _op_num;

  /// Order parameters
  std::vector<const VariableGradient *> _grads;

  /// normalization factor, depending on order parameter range and grains per side
  const Real _factor;
};

#endif // GRAINBOUNDARYAREA_H
