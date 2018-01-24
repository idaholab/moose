/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SIMPLECHINTERFACE_H
#define SIMPLECHINTERFACE_H

#include "Kernel.h"

class SimpleCHInterface;

template <>
InputParameters validParams<SimpleCHInterface>();

/**
 * Compute the Cahn-Hilliard interface term with constant Mobility and Interfacial parameter
 */
class SimpleCHInterface : public Kernel
{
public:
  SimpleCHInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  ///@{ Variables for second order derivatives
  const VariableSecond & _second_u;
  const VariableTestSecond & _second_test;
  const VariablePhiSecond & _second_phi;
  ///@}

  /// Mobility
  const MaterialProperty<Real> & _M;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa_c;
};

#endif // SIMPLECHINTERFACE_H
