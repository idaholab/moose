/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFULLYSATURATEDHEATADVECTION_H
#define POROUSFLOWFULLYSATURATEDHEATADVECTION_H

#include "PorousFlowFullySaturatedDarcyBase.h"

class PorousFlowFullySaturatedHeatAdvection;

template <>
InputParameters validParams<PorousFlowFullySaturatedHeatAdvection>();

/**
 * Advection of heat via flux via Darcy flow of a single phase
 * fully-saturated fluid.  No upwinding is used.
 */
class PorousFlowFullySaturatedHeatAdvection : public PorousFlowFullySaturatedDarcyBase
{
public:
  PorousFlowFullySaturatedHeatAdvection(const InputParameters & parameters);

protected:
  virtual Real mobility() const override;
  virtual Real dmobility(unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of the enthalpy wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;
};

#endif // POROUSFLOWFULLYSATURATEDHEATADVECTION_H
