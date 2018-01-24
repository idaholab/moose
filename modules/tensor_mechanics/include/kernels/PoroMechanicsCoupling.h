//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
