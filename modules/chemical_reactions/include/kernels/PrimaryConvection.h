/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Kernel.h"

#ifndef PRIMARYCONVECTION_H
#define PRIMARYCONVECTION_H

// Forward Declaration
class PrimaryConvection;

template <>
InputParameters validParams<PrimaryConvection>();

/**
 * Define the Kernel for a PrimaryConvection operator that looks like:
 * cond * grad_pressure * grad_u
 */
class PrimaryConvection : public Kernel
{
public:
  PrimaryConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Material property of hydraulic conductivity
  const MaterialProperty<Real> & _cond;

private:
  /// Coupled gradient of hydraulic head.
  const VariableGradient & _grad_p;
};

#endif // PRIMARYCONVECTION_H
