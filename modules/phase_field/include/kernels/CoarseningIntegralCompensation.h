/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COARSENINGINTEGRALCOMPENSATION_H
#define COARSENINGINTEGRALCOMPENSATION_H

#include "Kernel.h"

class CoarseningIntegralTracker;
class CoarseningIntegralCompensation;

template<>
InputParameters validParams<CoarseningIntegralCompensation>();

/**
 * Apply the corrective source term computed by the CoarseningIntegralTracker.
 */
class CoarseningIntegralCompensation : public Kernel
{
public:
  CoarseningIntegralCompensation(const InputParameters & parameters);

protected:
  virtual void precalculateResidual() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const CoarseningIntegralTracker & _tracker;

  // source for the current element
  Real _element_correction;

  // current timestep size
  const Real & _dt;
};

#endif // COARSENINGINTEGRALCOMPENSATION_H
