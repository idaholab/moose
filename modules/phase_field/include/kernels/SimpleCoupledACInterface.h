//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

/**
 * Compute the Allen-Cahn interface term with constant Mobility and Interfacial parameter
 */
template <bool is_ad>
class SimpleCoupledACInterfaceTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  SimpleCoupledACInterfaceTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Mobility
  const GenericMaterialProperty<Real, is_ad> & _L;
  /// Interfacial parameter
  const GenericMaterialProperty<Real, is_ad> & _kappa;
  /// Gradient of the coupled variable
  const GenericVariableGradient<is_ad> & _grad_v;

  usingGenericKernelMembers;
};

class SimpleCoupledACInterface : public SimpleCoupledACInterfaceTempl<false>
{
public:
  SimpleCoupledACInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Index of the coupled variable
  unsigned int _v_var;
};

using ADSimpleCoupledACInterface = SimpleCoupledACInterfaceTempl<true>;