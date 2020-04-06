//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDiffusivityBase.h"

/**
 * Material to provide saturation dependent diffusivity using the model of
 * Millington and Quirk, from
 * Millington and Quirk, Permeability of Porous Solids, Trans. Faraday Soc.,
 * 57, 1200- 1207, 1961
 */
class PorousFlowDiffusivityMillingtonQuirk : public PorousFlowDiffusivityBase
{
public:
  static InputParameters validParams();

  PorousFlowDiffusivityMillingtonQuirk(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Porosity at the qps
  const MaterialProperty<Real> & _porosity_qp;
  /// Derivative of porosity wrt PorousFlow variables (at the qps)
  const MaterialProperty<std::vector<Real>> & _dporosity_qp_dvar;
  /// Saturation of each phase at the qps
  const MaterialProperty<std::vector<Real>> & _saturation_qp;
  /// Derivative of saturation of each phase wrt PorousFlow variables (at the qps)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_qp_dvar;
};
