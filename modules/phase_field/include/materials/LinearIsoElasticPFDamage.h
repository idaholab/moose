/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARISOELASTICPFDAMAGE_H
#define LINEARISOELASTICPFDAMAGE_H

#include "LinearElasticMaterial.h"
#include "Function.h"

/**
 * Phase-field fracture
 * This class computes the energy contribution to damage growth
 * Small strain Isotropic Elastic formulation
 * Stiffness matrix scaled for heterogeneous elasticity property
 */
class LinearIsoElasticPFDamage : public LinearElasticMaterial
{
public:
  LinearIsoElasticPFDamage(const InputParameters & parameters);
  LinearIsoElasticPFDamage(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual void computeQpElasticityTensor();
  virtual void computeQpStress();
  virtual void updateVar();

  /**
   * This function obtains the value of _scaling
   * Must be overidden by the user for heterogeneous elasticity
   */
  virtual void getScalingFactor();

  VariableValue & _c;
  Real _kdamage;

  MaterialProperty<RankTwoTensor> & _dstress_dc;
  MaterialProperty<Real> & _G0_pos;
  MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;

  /// Small number to avoid non-positive definiteness at or near complete damage
  Real _scaling;
  std::vector<RankTwoTensor> _etens;
  std::vector<Real> _epos;
};

#endif //LINEARISOELASTICPFDAMAGE_H
