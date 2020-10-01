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

// Forward Declarations

/**
 * Compute the gradient interface terms for a multiphase system. This includes
 * cross terms of the form \f$ \left( \eta_a\nabla\eta_b - \eta_b\nabla\eta_a \right)^2\f$.
 * Note that in a two phase system with \f$ \eta = \eta_b = 1-\eta_a \f$ the regular ACInterface
 * kernel can be used as the gradient interface term simplifies to the usual
 * \f$ ()\nabla\eta)^2 \f$ form.
 *
 * http://mooseframework.org/wiki/PhysicsModules/PhaseField/DevelopingModels/MultiPhaseModels/ACMultiInterface/
 */
class ACMultiInterface : public Kernel
{
public:
  static InputParameters validParams();

  ACMultiInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Order parameters
  const unsigned int _num_etas;
  const std::vector<const VariableValue *> _eta;
  const std::vector<const VariableGradient *> _grad_eta;

  /// Lookup table from couple variable number into the etas vector
  std::vector<int> _eta_vars;

  /// Index of the eta this kernel is operating on
  unsigned int _a;

  /// Interface gradient prefactor
  std::vector<MaterialPropertyName> _kappa_names;
  std::vector<const MaterialProperty<Real> *> _kappa;

  /// Mobility
  const MaterialProperty<Real> & _L;
};
