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
template <bool is_ad>
class PorousFlowDiffusivityBaseTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowDiffusivityBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  GenericMaterialProperty<std::vector<Real>, is_ad> & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> * const _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _ddiffusion_coeff_dvar;

  /// Input diffusion coefficients
  const std::vector<Real> _input_diffusion_coeff;
};

#define usingPorousFlowDiffusivityBaseMembers                                                      \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_num_phases;                                        \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_num_var;                                           \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_tortuosity;                                        \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_qp;                                                \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_dtortuosity_dvar;                                  \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_diffusion_coeff;                                   \
  using PorousFlowDiffusivityBaseTempl<is_ad>::_ddiffusion_coeff_dvar
