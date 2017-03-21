/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARISOELASTICPFDAMAGE_H
#define LINEARISOELASTICPFDAMAGE_H

#include "ComputeStressBase.h"
#include "Function.h"

/**
 * Phase-field fracture
 * This class computes the energy contribution to damage growth
 * Small strain Isotropic Elastic formulation
 * Stiffness matrix scaled for heterogeneous elasticity property
 */
class LinearIsoElasticPFDamage : public ComputeStressBase
{
public:
  LinearIsoElasticPFDamage(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void updateVar();
  virtual void updateJacobian();

  const VariableValue & _c;
  /// Small number to avoid non-positive definiteness at or near complete damage
  Real _kdamage;

  MaterialProperty<Real> & _G0_pos;
  MaterialProperty<RankTwoTensor> & _dstress_dc;
  MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;

  std::vector<RankTwoTensor> _etens;
  std::vector<Real> _epos;
  std::vector<Real> _eigval;
  RankTwoTensor _eigvec;
};

#endif // LINEARISOELASTICPFDAMAGE_H
