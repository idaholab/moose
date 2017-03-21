/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SWITCHINGFUNCTIONCONSTRAINTETA_H
#define SWITCHINGFUNCTIONCONSTRAINTETA_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class SwitchingFunctionConstraintEta;

template <>
InputParameters validParams<SwitchingFunctionConstraintEta>();

/**
 * SwitchingFunctionConstraintEta is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class SwitchingFunctionConstraintEta : public DerivativeMaterialInterface<Kernel>
{
public:
  SwitchingFunctionConstraintEta(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function name
  VariableName _eta_name;

  /// Switching function drivatives
  const MaterialProperty<Real> & _dh;
  const MaterialProperty<Real> & _d2h;

  /// Lagrange multiplier
  const VariableValue & _lambda;
  unsigned int _lambda_var;
};

#endif // SWITCHINGFUNCTIONCONSTRAINTETA_H
