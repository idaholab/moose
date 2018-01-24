/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWHEATADVECTION_H
#define POROUSFLOWHEATADVECTION_H

#include "PorousFlowDarcyBase.h"

class PorousFlowHeatAdvection;

template <>
InputParameters validParams<PorousFlowHeatAdvection>();

/**
 * Advection of heat via flux of component k in fluid phase alpha.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowHeatAdvection : public PorousFlowDarcyBase
{
public:
  PorousFlowHeatAdvection(const InputParameters & parameters);

protected:
  virtual Real mobility(unsigned nodenum, unsigned phase) const override;
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of the enthalpy wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;
};

#endif // POROUSFLOWHEATADVECTION_H
