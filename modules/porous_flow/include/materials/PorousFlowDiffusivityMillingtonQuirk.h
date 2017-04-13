/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDIFFUSIVITYMILLINGTONQUIRK_H
#define POROUSFLOWDIFFUSIVITYMILLINGTONQUIRK_H

#include "PorousFlowDiffusivityBase.h"

class PorousFlowDiffusivityMillingtonQuirk;

template <>
InputParameters validParams<PorousFlowDiffusivityMillingtonQuirk>();

/**
 * Material to provide saturation dependent diffusivity using the model of
 * Millington and Quirk, from
 * Millington and Quirk, Permeability of Porous Solids, Trans. Faraday Soc.,
 * 57, 1200- 1207, 1961
 */
class PorousFlowDiffusivityMillingtonQuirk : public PorousFlowDiffusivityBase
{
public:
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

#endif // POROUSFLOWDIFFUSIVITYMILLINGTONQUIRK_H
