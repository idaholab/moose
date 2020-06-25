//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * Compute the Allen-Cahn interface term with the weak form residual
 * \f$ \left( \kappa_i \nabla\eta_i, \nabla (L_i \psi) \right) \f$
 */
class ADACInterface : public ADKernel, public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  ADACInterface(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /// Mobility
  const ADMaterialProperty<Real> & _prop_L;
  /// Mobility property name
  const MaterialPropertyName & _name_L;

  /// Interfacial parameter
  const ADMaterialProperty<Real> & _kappa;

  /// flag set if L is a function of non-linear variables in args
  const bool _variable_L;

  /// Mobility derivative w.r.t. order parameter
  const ADMaterialProperty<Real> * const _dLdop;

  /// number of coupled variables
  const unsigned int _nvar;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const ADMaterialProperty<Real> *> _dLdarg;

  /// Gradients for all coupled variables
  std::vector<const ADVariableGradient *> _gradarg;
};
