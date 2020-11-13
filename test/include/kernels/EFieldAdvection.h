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

class EFieldAdvection : public Kernel
{
public:
  static InputParameters validParams();

  EFieldAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const unsigned int _efield_id;
  const VectorVariableValue & _efield;
  const bool _efield_coupled;
  const VectorMooseVariable * const _efield_var;
  const VectorVariablePhiValue * const _vector_phi;
  const Real _mobility;
  const Real _sgn;
};
