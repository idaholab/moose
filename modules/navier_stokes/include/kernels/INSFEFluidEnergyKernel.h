//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidKernelStabilization.h"

#include "Function.h"

/**
 * The spatial part of the 3D energy conservation for fluid flow
 */
class INSFEFluidEnergyKernel : public INSFEFluidKernelStabilization
{
public:
  static InputParameters validParams();

  INSFEFluidEnergyKernel(const InputParameters & parameters);
  virtual ~INSFEFluidEnergyKernel() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  bool _conservative_form;
  const MaterialProperty<Real> & _k_elem;
  const MaterialProperty<Real> & _cp;
  bool _has_porosity_elem;
  const VariableValue & _porosity_elem;
  bool _has_qv;
  const VariableValue & _qv; /// volumetric heat source
  bool _has_pke;
  const VariableValue & _pke_qv;
  const Function * _power_shape_function;
};
