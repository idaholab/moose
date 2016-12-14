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

template<>
  InputParameters validParams<EqualGradientLagrangeMultiplier>();

/**
 * Lagrange multiplier "FaceKernel" that is used in conjunction with EqualGradientLagrangeInterface.
 */
class EqualGradientLagrangeMultiplier : public InterfaceKernel
{
public:
  EqualGradientLagrangeMultiplier(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  const unsigned int _component;

  const VariableGradient & _grad_element_value;
  unsigned int _element_jvar;
  unsigned int _neighbor_jvar;
};


#endif //EQUALGRADIENTLAGRANGEMULTIPLIER_H
