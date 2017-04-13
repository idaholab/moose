/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTHERMALCONDUCTIVITYIDEAL_H
#define POROUSFLOWTHERMALCONDUCTIVITYIDEAL_H

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowThermalConductivityIdeal;

template <>
InputParameters validParams<PorousFlowThermalConductivityIdeal>();

/**
 * This material computes thermal conductivity for a PorousMedium - fluid
 * system, by using
 * Thermal conductivity = dry_thermal_conductivity + S^exponent * (wet_thermal_conductivity -
 * dry_thermal_conductivity),
 * where S is the aqueous saturation.
 */
class PorousFlowThermalConductivityIdeal : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowThermalConductivityIdeal(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Dry thermal conductivity of rock
  const RealTensorValue _la_dry;

  /// Whether _la_wet has been supplied
  const bool _wet_and_dry_differ;

  /// Wet thermal conductivity of rock
  const RealTensorValue _la_wet;

  /// exponent for saturation
  const Real _exponent;

  /// whether this is a fluid simulation
  const bool _aqueous_phase;

  /// Phase number of the aqueous phase
  const unsigned _aqueous_phase_number;

  /// Saturation of the fluid phases at the quadpoints
  const MaterialProperty<std::vector<Real>> * const _saturation_qp;

  /// d(Saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dsaturation_qp_dvar;

  /// Thermal conducitivity at the qps
  MaterialProperty<RealTensorValue> & _la_qp;

  /// d(thermal conductivity at the qps)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue>> & _dla_qp_dvar;
};

#endif // POROUSFLOWTHERMALCONDUCTIVITYIDEAL_H
