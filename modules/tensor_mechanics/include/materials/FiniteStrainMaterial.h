/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: M.R. Tonks

#ifndef FINITESTRAINMATERIAL_H
#define FINITESTRAINMATERIAL_H

#include "TensorMechanicsMaterial.h"

/**
 * FiniteStrainMaterial implements a finite strain formulation using the tensor mechanics system.
 * It determines the strain increment, the strain rate and the rotation increment. It uses the
 * form from Rashid, 1993 in which the constitutive calculation is conducted in an intermediate
 * configruation. The final stress is then rotated via the incremental rotation to the current
 * cofiguration.
 *
 * Any material that inherits from this one must override ComputeQpStress with the desired
 * constitutive relationship.
 */
class FiniteStrainMaterial : public TensorMechanicsMaterial
{
public:
  FiniteStrainMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeStrain();
  virtual void computeQpStrain();
  virtual void computeQpStrain(const RankTwoTensor & Fhat);
  virtual void computeQpStress() = 0;

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  MaterialProperty<RankTwoTensor> & _stress_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;
};

#endif // FINITESTRAINMATERIAL_H
