/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SIMPLEACINTERFACE_H
#define SIMPLEACINTERFACE_H

#include "Kernel.h"

class SimpleACInterface;

template <>
InputParameters validParams<SimpleACInterface>();

/**
 * Compute the Allen-Cahn interface term with constant Mobility and Interfacial parameter
 */
class SimpleACInterface : public Kernel
{
public:
  SimpleACInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Mobility
  const MaterialProperty<Real> & _L;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;
};

#endif // SIMPLEACINTERFACE_H
