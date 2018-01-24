/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARRATECOMPONENTGSS_H
#define CRYSTALPLASTICITYSTATEVARRATECOMPONENTGSS_H

#include "CrystalPlasticityStateVarRateComponent.h"

class CrystalPlasticityStateVarRateComponentGSS;

template <>
InputParameters validParams<CrystalPlasticityStateVarRateComponentGSS>();

/**
 * Phenomenological constitutive model state variable evolution rate component userobject class.
 */
class CrystalPlasticityStateVarRateComponentGSS : public CrystalPlasticityStateVarRateComponent
{
public:
  CrystalPlasticityStateVarRateComponentGSS(const InputParameters & parameters);

  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const;

protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_slip_rate;
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

  /// The hardening parameters in this class are read from .i file. The user can override to read from file.
  FileName _slip_sys_hard_prop_file_name;

  std::vector<Real> _hprops;
};

#endif // CRYSTALPLASTICITYSTATEVARRATECOMPONENTGSS_H
