/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWEXPONENTIALDECAY_H
#define POROUSFLOWEXPONENTIALDECAY_H

#include "Kernel.h"

// Forward Declarations
class PorousFlowExponentialDecay;

template <>
InputParameters validParams<PorousFlowExponentialDecay>();

/**
 * Kernel = _rate * (variable - reference)
 */
class PorousFlowExponentialDecay : public Kernel
{
public:
  PorousFlowExponentialDecay(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// The decay rate
  const VariableValue & _rate;

  /// The reference
  const VariableValue & _reference;
};

#endif // POROUSFLOWEXPONENTIALDECAY_H
