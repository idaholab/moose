/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENTGSS_H
#define CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENTGSS_H

#include "CrystalPlasticityStateVariableEvolutionRateComponent.h"

class CrystalPlasticityStateVariableEvolutionRateComponentGSS;

template<>InputParameters validParams<CrystalPlasticityStateVariableEvolutionRateComponentGSS>();

/**
 * Phenomenological constitutive model state variable evolution rate component userobject class.
 */
class CrystalPlasticityStateVariableEvolutionRateComponentGSS : public CrystalPlasticityStateVariableEvolutionRateComponent
{
 public:
  CrystalPlasticityStateVariableEvolutionRateComponentGSS(const InputParameters & parameters);

  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp, std::vector<Real> & val) const;

 protected:
  const MaterialProperty<std::vector<Real> > &  _mat_prop_slip_rate;
  const MaterialProperty<std::vector<Real> > & _mat_prop_state_var;

  std::vector<Real> _hprops;
};

#endif // CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENTGSS_H
