//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

/**
 * PoroMechanicsCoupling computes -coefficient*porepressure*grad_test[component]
 */
class PoroMechanicsCoupling : public Kernel
{
public:
  static InputParameters validParams();

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

  /// An integer corresponding to the direction this kernel acts in
  unsigned int _component;
};
