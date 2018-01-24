/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHSPLITFLUX_H
#define CHSPLITFLUX_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

class CHSplitFlux;

template <>
InputParameters validParams<CHSplitFlux>();

/**
 * CHSplitFlux computes flux as non-linear variable via
 * residual = flux + mobility * gradient(chemical potential)
 * Kernel is associated with a component (direction) that needs to be specified in the input file
 */
class CHSplitFlux : public DerivativeMaterialInterface<Kernel>
{
public:
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

  const MaterialProperty<RealTensorValue> * _dmobility_dc;
};

#endif
