//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IMPLICITODEX_H
#define IMPLICITODEX_H

#include "ODEKernel.h"

// Forward Declarations
class ImplicitODEx;

template <>
InputParameters validParams<ImplicitODEx>();

/**
 *
 */
class ImplicitODEx : public ODEKernel
{
public:
  ImplicitODEx(const InputParameters & parameters);
  virtual ~ImplicitODEx();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _y_var;
  VariableValue & _y;
};

#endif /* IMPLICITODEX_H */
