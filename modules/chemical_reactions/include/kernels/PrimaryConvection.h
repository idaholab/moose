/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PRIMARYCONVECTION_H
#define PRIMARYCONVECTION_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class PrimaryConvection;

template <>
InputParameters validParams<PrimaryConvection>();

/**
 * Define the Kernel for a PrimaryConvection operator that looks like:
 * cond * grad_pressure * grad_u
 */
class PrimaryConvection : public DerivativeMaterialInterface<Kernel>
{
public:
  PrimaryConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Hydraulic conductivity
  const MaterialProperty<Real> & _cond;

  /// Gravity
  const RealVectorValue _gravity;

  /// Fluid density
  const MaterialProperty<Real> & _density;

  /// Pressure gradient
  const VariableGradient & _grad_p;

  /// Pressure variable number
  const unsigned int _pvar;
};

#endif // PRIMARYCONVECTION_H
