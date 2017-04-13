/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWSINKPTDEFINER_H
#define POROUSFLOWSINKPTDEFINER_H

#include "PorousFlowSink.h"

// Forward Declarations
class PorousFlowSinkPTDefiner;

template <>
InputParameters validParams<PorousFlowSinkPTDefiner>();

/**
 * Provides either a porepressure or a temperature
 * to derived classes, depending on _involves_fluid
 * defined in PorousFlowSink
 */
class PorousFlowSinkPTDefiner : public PorousFlowSink
{
public:
  PorousFlowSinkPTDefiner(const InputParameters & parameters);

protected:
  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real>> * const _pp;

  /// d(Nodal pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dpp_dvar;

  /// Nodal temperature
  const MaterialProperty<Real> * const _temp;

  /// d(Nodal temperature)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dtemp_dvar;

  /// Provides the variable value (either porepressure, or temperature, depending on _involves_fluid)
  virtual Real ptVar() const;

  /// Provides the d(variable)/(d PorousFlow Variable pvar)
  virtual Real dptVar(unsigned pvar) const;
};

#endif // POROUSFLOWSINKPTDEFINER_H
