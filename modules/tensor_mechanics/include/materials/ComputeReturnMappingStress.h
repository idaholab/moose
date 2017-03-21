/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERETURNMAPPINGSTRESS_H
#define COMPUTERETURNMAPPINGSTRESS_H

#include "ComputeFiniteStrainElasticStress.h"

class StressUpdateBase;

/**
 * ComputeReturnMappingStress computes the stress, with a return mapping
 * stress increment following elasticity theory for finite strains. The elastic
 * strain is calculated by subtracting the return mapping inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials, and, if two
 * over more recompute radial return materials are specified, iterates
 * over the change in the total effective (scalar) radial return stress between
 * loops over all of the recompute materials: once the change in stress is within
 * a user-specified tolerance, this class calculates the final stress and elastic
 * strain for the time increment.
 */

class ComputeReturnMappingStress : public ComputeFiniteStrainElasticStress
{
public:
  ComputeReturnMappingStress(const InputParameters & parameters);

protected:
  virtual void initialSetup();

  virtual void computeQpStress();

  /// Calls all of the user-specified radial recompute materials and iterates
  /// over the change in the effective radial return stress.
  virtual void updateQpStress(RankTwoTensor & strain_increment, RankTwoTensor & stress_new);

  ///@{Input parameters associated with the recompute iteration to return the stress state to the yield surface
  const unsigned int _max_its;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
  const bool _output_iteration_info;
  ///@}

  ///@{ Rank-4 and Rank-2 elasticity and elastic strain tensors
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  ///@}

  /// The user supplied list of recompute radial return models to use in the
  /// simulation.  Users should take care to list creep models first and plasticity
  // models last to allow for the case when a creep model relaxes the stress state
  /// inside of the yield surface in an iteration.
  std::vector<StressUpdateBase *> _models;
};

#endif // COMPUTERETURNMAPPINGSTRESS_H
