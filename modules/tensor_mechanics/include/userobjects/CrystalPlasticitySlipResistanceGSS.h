/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSLIPRESISTANCEGSS_H
#define CRYSTALPLASTICITYSLIPRESISTANCEGSS_H

#include "CrystalPlasticitySlipResistance.h"

class CrystalPlasticitySlipResistanceGSS;

template <>
InputParameters validParams<CrystalPlasticitySlipResistanceGSS>();

/**
 * Phenomenological constitutive model slip resistance userobject class.
 */
class CrystalPlasticitySlipResistanceGSS : public CrystalPlasticitySlipResistance
{
public:
  CrystalPlasticitySlipResistanceGSS(const InputParameters & parameters);

  virtual bool calcSlipResistance(unsigned int qp, std::vector<Real> & val) const;

protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;
};

#endif // CRYSTALPLASTICITYSLIPRESISTANCEGSS_H
