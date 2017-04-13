/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SIMPLECOUPLEDACINTERFACE_H
#define SIMPLECOUPLEDACINTERFACE_H

#include "Kernel.h"

class SimpleCoupledACInterface;

template <>
InputParameters validParams<SimpleCoupledACInterface>();

/**
 * Compute the Allen-Cahn interface term with constant Mobility and Interfacial parameter
 */
class SimpleCoupledACInterface : public Kernel
{
public:
  SimpleCoupledACInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;
  /// Gradient of the coupled variable
  const VariableGradient & _grad_v;
  /// Index of the coupled variable
  unsigned int _v_var;
};

#endif // SIMPLECOUPLEDACINTERFACE_H
