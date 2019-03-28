//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEEXTRASTRESSVDWGAS_H
#define COMPUTEEXTRASTRESSVDWGAS_H

#include "ComputeExtraStressBase.h"

class ComputeExtraStressVDWGas;

template <>
InputParameters validParams<ComputeExtraStressVDWGas>();

/**
 * Computes a concentration-dependent ExtraStress bases on the van der Waals
 * equation of state that is added to the stress computed by the constitutive model
 */
class ComputeExtraStressVDWGas : public ComputeExtraStressBase
{
public:
  ComputeExtraStressVDWGas(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();

  const MaterialProperty<Real> & _b;
  const MaterialProperty<Real> & _Va;
  const MaterialProperty<Real> & _T;

  const VariableValue & _cg;
  const Real _nondim_factor;
  const Real _kB;
};

#endif // COMPUTEEXTRASTRESSVDWGAS_H
