/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHSPLITCONCENTRATION_H
#define CHSPLITCONCENTRATION_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class CHSplitConcentration;

template <>
InputParameters validParams<CHSplitConcentration>();

/**
 * Solves Cahn-Hilliard equation using
 * chemical potential as non-linear variable
 **/
class CHSplitConcentration : public DerivativeMaterialInterface<Kernel>
{
public:
  CHSplitConcentration(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Mobility property name
  MaterialPropertyName _mobility_name;

  const MaterialProperty<RealTensorValue> & _mobility;
  const MaterialProperty<RealTensorValue> & _dmobility_dc;

  /// Chemical potential variable
  const unsigned int _mu_var;
  const VariableGradient & _grad_mu;
};

#endif
