/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EQUALGRADIENTLAGRANGEMULTIPLIER_H
#define EQUALGRADIENTLAGRANGEMULTIPLIER_H

#include "InterfaceKernel.h"

class EqualGradientLagrangeMultiplier;

template <>
InputParameters validParams<EqualGradientLagrangeMultiplier>();

/**
 * Lagrange multiplier "FaceKernel" that is used in conjunction with
 * EqualGradientLagrangeInterface.
 */
class EqualGradientLagrangeMultiplier : public InterfaceKernel
{
public:
  EqualGradientLagrangeMultiplier(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  /// x,y,z component of the gradient to constrain
  const unsigned int _component;

  ///@{ variable to control gradient on the master side of the interface
  const VariableGradient & _grad_element_value;
  unsigned int _element_jvar;
  ///@}

  /// variable to control gradient on the slave side of the interface
  unsigned int _neighbor_jvar;

  /// compensate Jacobian fill term from NullKernel
  const Real _jacobian_fill;
};

#endif // EQUALGRADIENTLAGRANGEMULTIPLIER_H
