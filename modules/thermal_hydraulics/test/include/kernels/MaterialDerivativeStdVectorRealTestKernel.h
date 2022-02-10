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
 * Kernel for testing derivatives of a std::vector<Real> material property
 */
class MaterialDerivativeStdVectorRealTestKernel
  : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  MaterialDerivativeStdVectorRealTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// number of nonlinear variables
  const unsigned int _n_vars;

  const std::string & _name;

  /// material property for which to test derivatives
  const MaterialProperty<Real> & _p;

  /// material properties for the off-diagonal derivatives of the tested property
  std::vector<const MaterialProperty<std::vector<Real>> *> _p_off_diag_derivatives;

  /// material property for the diagonal derivative of the tested property
  const MaterialProperty<std::vector<Real>> & _p_diag_derivative;

  /// the component of the std::vector material property
  const unsigned int _component_i;

public:
  static InputParameters validParams();
};
