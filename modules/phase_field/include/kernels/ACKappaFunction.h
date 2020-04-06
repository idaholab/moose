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
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * When kappa is a function of phase field variables, this kernel should be used
 * to calculate the term which includes the derivatives of kappa.
 **/

class ACKappaFunction : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  ACKappaFunction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdvar;

  const MaterialPropertyName _kappa_name;
  const MaterialProperty<Real> & _dkappadvar;
  const MaterialProperty<Real> & _d2kappadvar2;

  const unsigned int _v_num;
  JvarMap _v_map;

  std::vector<const VariableGradient *> _grad_v;
  std::vector<const MaterialProperty<Real> *> _dLdv;
  std::vector<const MaterialProperty<Real> *> _d2kappadvardv;

private:
  Real computeFg(); /// gradient energy term
};
