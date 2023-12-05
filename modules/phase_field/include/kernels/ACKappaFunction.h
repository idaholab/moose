//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * When kappa is a function of phase field variables, this kernel should be used
 * to calculate the term which includes the derivatives of kappa.
 **/

template <bool is_ad>
class ACKappaFunctionTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>
{
public:
  static InputParameters validParams();

  ACKappaFunctionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  const GenericMaterialProperty<Real, is_ad> & _L;

  const MaterialPropertyName _kappa_name;
  const GenericMaterialProperty<Real, is_ad> & _dkappadvar;

  const unsigned int _v_num;
  std::vector<const GenericVariableGradient<is_ad> *> _grad_v;

  GenericReal<is_ad> computeFg(); /// gradient energy term

  using GenericKernel<is_ad>::_qp;
  using GenericKernel<is_ad>::_i;
  using GenericKernel<is_ad>::_grad_u;
  using GenericKernel<is_ad>::_var;
  using GenericKernel<is_ad>::_test;
};

class ACKappaFunction : public ACKappaFunctionTempl<false>
{
public:
  ACKappaFunction(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const MaterialProperty<Real> & _dLdvar;
  const MaterialProperty<Real> & _d2kappadvar2;

  JvarMap _v_map;
  std::vector<const MaterialProperty<Real> *> _dLdv;
  std::vector<const MaterialProperty<Real> *> _d2kappadvardv;
};

typedef ACKappaFunctionTempl<true> ADACKappaFunction;
