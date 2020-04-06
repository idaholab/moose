//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/// Base class Material designed to provide the tortuosity and diffusion coefficents
class PorousFlowDiffusivityBase : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowDiffusivityBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  MaterialProperty<std::vector<Real>> & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _ddiffusion_coeff_dvar;

  /// Input diffusion coefficients
  const std::vector<Real> _input_diffusion_coeff;
};
