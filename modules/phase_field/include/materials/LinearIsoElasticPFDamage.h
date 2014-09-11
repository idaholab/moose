/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
  LinearIsoElasticPFDamage(const std:: string & name, InputParameters parameters);

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
  ///Small number to avoid non-positive definiteness at or near complete damage

  Real _scaling;
  std::vector<RankTwoTensor> _etens;
  std::vector<Real> _epos;
};

#endif //LINEARISOELASTICPFDAMAGE_H
