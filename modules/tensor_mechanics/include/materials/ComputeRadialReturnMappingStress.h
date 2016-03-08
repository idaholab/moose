/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERADIALRETURNMAPPINGSTRESS_H
#define COMPUTERADIALRETURNMAPPINGSTRESS_H

#include "ComputeFiniteStrainElasticStress.h"

/**
 * ComputeRadialReturnMappingStress computes the stress following elasticity
 * theory for finite strains after subtracting the radial return strain increment.
 * The radial return mapping strain increment is computed by DiscreteRadialReturnStressIncrement
 * (or a class which inherits from DiscreteRadialReturnStressIncrement) following
 * the algorithms from Dunne and Petrinic's Introduction to Computational Plasticity.
 */

class ComputeRadialReturnMappingStress : public ComputeFiniteStrainElasticStress
{
public:
  ComputeRadialReturnMappingStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  const MaterialProperty<RankTwoTensor> & _radial_return_stress;
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_increment;
  DiscreteMaterial & _discrete_radial_return_material;
};

#endif //COMPUTERADIALRETURNMAPPINGSTRESS_H
