/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROMECHANICSCOUPLING_H
#define POROMECHANICSCOUPLING_H

#include "Kernel.h"

// Forward Declarations
class PoroMechanicsCoupling;

template <>
InputParameters validParams<PoroMechanicsCoupling>();

/**
 * PoroMechanicsCoupling computes -coefficient*porepressure*grad_test[component]
 */
class PoroMechanicsCoupling : public Kernel
{
public:
  PoroMechanicsCoupling(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Biot coefficient
  const MaterialProperty<Real> & _coefficient;

  const VariableValue & _porepressure;

  unsigned int _porepressure_var_num;

  unsigned int _component;
};

#endif // POROMECHANICSCOUPLING_H
