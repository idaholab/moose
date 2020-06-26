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
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

/**
 * CHSplitFlux computes flux as non-linear variable via
 * residual = flux + mobility * gradient(chemical potential)
 * Kernel is associated with a component (direction) that needs to be specified in the input file
 */
class CHSplitFlux : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  CHSplitFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const unsigned int _component;
  const unsigned int _mu_var;
  const VariableGradient & _grad_mu;
  const MaterialProperty<RealTensorValue> & _mobility;

  const bool _has_coupled_c;
  const unsigned int _c_var;

  const MaterialProperty<RealTensorValue> * const _dmobility_dc;
};
