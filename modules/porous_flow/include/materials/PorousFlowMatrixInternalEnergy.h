/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATRIXINTERNALENERGY_H
#define POROUSFLOWMATRIXINTERNALENERGY_H

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowMatrixInternalEnergy;

template <>
InputParameters validParams<PorousFlowMatrixInternalEnergy>();

/**
 * This material computes internal energy (J/m^3) for a rock matrix
 * assuming constant grain density, specific heat capacity, and
 * a linear relationship with temperature.  To get the volumetric
 * heat capacity of the rock in a rock-fluid system, the result must
 * be multiplied by (1 - porosity).
 */
class PorousFlowMatrixInternalEnergy : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowMatrixInternalEnergy(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Specific heat capacity of rock grains
  const Real _cp;

  /// Density of rock grains (equals the density of the matrix if porosity=0)
  const Real _density;

  /// Heat capacity = _cp * _density
  const Real _heat_cap;

  /// temperature at the nodes
  const MaterialProperty<Real> & _temperature_nodal;

  /// d(temperature at the nodes)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dtemperature_nodal_dvar;

  /// Matrix internal_energy at the nodes
  MaterialProperty<Real> & _en_nodal;

  /// d(matrix internal energy)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _den_nodal_dvar;
};

#endif // POROUSFLOWMATRIXINTERNALENERGY_H
