/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARELASTICPFFRACTURESTRESS_H
#define COMPUTELINEARELASTICPFFRACTURESTRESS_H

#include "ComputeStressBase.h"

/**
 * Phase-field fracture
 * This class computes the stress and energy contribution to fracture
 * Small strain Anisotropic Elastic formulation
 * Stiffness matrix scaled for heterogeneous elasticity property
 */
class ComputeLinearElasticPFFractureStress : public ComputeStressBase
{
public:
  ComputeLinearElasticPFFractureStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  const VariableValue & _c;

  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  /// Characteristic length, controls damage zone thickness
  const MaterialProperty<Real> & _l;
  /// Small number to avoid non-positive definiteness at or near complete damage
  Real _kdamage;

  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  MaterialProperty<Real> & _H0_pos;
  MaterialProperty<Real> & _H0_pos_old; // History variable from previous timestep
};

#endif // COMPUTELINEARELASTICPFFRACTURESTRESS_H
