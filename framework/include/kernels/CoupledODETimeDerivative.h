//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDODETIMEDERIVATIVE_H
#define COUPLEDODETIMEDERIVATIVE_H

#include "ODETimeKernel.h"

// Forward Declaration
class CoupledODETimeDerivative;

template <>
InputParameters validParams<CoupledODETimeDerivative>();

class CoupledODETimeDerivative : public ODETimeKernel
{
public:
  CoupledODETimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  VariableValue & _v_dot;
  VariableValue & _dv_dot_dv;
};

#endif // ODETIMEDERIVATIVE_H
