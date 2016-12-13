/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEGRADIENTMATCH_H
#define INTERFACEGRADIENTMATCH_H

#include "InterfaceKernel.h"

class InterfaceGradientMatch;

template<>
InputParameters validParams<InterfaceGradientMatch>();

/**
 *
 */
class InterfaceGradientMatch : public InterfaceKernel
{
public:
  InterfaceGradientMatch(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const unsigned int _component;
};

#endif // INTERFACEGRADIENTMATCH_H
