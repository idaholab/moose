/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROUSFLOWPLASTICHEATENERGY_H
#define POROUSFLOWPLASTICHEATENERGY_H

#include "PlasticHeatEnergy.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowPlasticHeatEnergy;

template <>
InputParameters validParams<PorousFlowPlasticHeatEnergy>();

/**
 * Provides a heat source (J/m^3/s) from plastic deformation:
 * (1 - porosity) * coeff * stress * plastic_strain_rate
 */
class PorousFlowPlasticHeatEnergy : public PlasticHeatEnergy
{
public:
  PorousFlowPlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// the nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// porosity at the nodes, but it can depend on grad(variables) which are actually evaluated at the qps
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(porous-flow variable) - these derivatives will be wrt variables at the nodes
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad porous-flow variable) - remember these derivatives will be wrt grad(vars) at qps
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;
};

#endif // POROUSFLOWPLASTICHEATENERGY_H
