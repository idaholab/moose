/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EQUALGRADIENTLAGRANGEINTERFACE_H
#define EQUALGRADIENTLAGRANGEINTERFACE_H

#include "InterfaceKernel.h"

class EqualGradientLagrangeInterface;

template <>
InputParameters validParams<EqualGradientLagrangeInterface>();

/**
 * InterfaceKernel to enforce a Lagrange-Multiplier based componentwise
 * continuity of a variable gradient.
 */
class EqualGradientLagrangeInterface : public InterfaceKernel
{
public:
  EqualGradientLagrangeInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  const unsigned int _component;

  /// Lagrange multiplier
  const VariableValue & _lambda;

  const unsigned int _lambda_jvar;
};

#endif // EQUALGRADIENTLAGRANGEINTERFACE_H
