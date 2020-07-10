//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatSource.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This kernel calculates the heat source term corresponding to joule heating,
 * Q = J * E = elec_cond * grad_phi * grad_phi, where phi is the electrical potential.
 */
class JouleHeatingSource : public DerivativeMaterialInterface<JvarMapKernelInterface<HeatSource>>
{
public:
  static InputParameters validParams();

  JouleHeatingSource(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const VariableGradient & _grad_elec;
  const unsigned int _elec_var;

  const MaterialProperty<Real> & _elec_cond;
  const MaterialProperty<Real> & _delec_cond_dT;
  std::vector<const MaterialProperty<Real> *> _delec_cond_darg;
};
