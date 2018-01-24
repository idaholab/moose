//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINEARVISCOELASTICSTRESSUPDATE_H
#define LINEARVISCOELASTICSTRESSUPDATE_H

#include "StressUpdateBase.h"
#include "LinearViscoelasticityBase.h"

class LinearViscoelasticStressUpdate;

template <>
InputParameters validParams<LinearViscoelasticStressUpdate>();

/**
 * This class computes a creep strain increment associated with a linear viscoelastic
 * model contained in a LinearViscoelasticityBase material. The creep strain increment
 * is deduced from the elastic strain increment and added to the inelastic strain increment.
 * The stress is reduced accordingly.
 *
 * This material must be used in conjunction with ComputeMultipleInelasticStress, and uses
 * an incremental (small or finite) strain formulation. To use viscoelastic models with
 * total small strain, use a ComputeLinearViscoelasticStress material instead.
 */
class LinearViscoelasticStressUpdate : public StressUpdateBase
{
public:
  LinearViscoelasticStressUpdate(const InputParameters & parameters);

  /**
   * Computes the new creep strain, and removes the creep contribution from the elastic strains and
   * stress. The tangent_operator is set equal to the elasticity tensor of the material.
   */
  virtual void updateState(RankTwoTensor & strain_increment,
                           RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & rotation_increment,
                           RankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const RankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;

  /// Reimplemented from StressUpdateBase
  virtual void propagateQpStatefulProperties() override;

  virtual bool requiresIsotropicTensor() override { return false; }

protected:
  virtual void initQpStatefulProperties() override;

  std::string _base_name;

  /// Creep strain
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  /// Apparent creep strain (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankTwoTensor> & _apparent_creep_strain;
  /// Apparent elasticity tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  /// Instantaneous compliance tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor_inv;
};

#endif // LINEARVISCOELASTICSTRESSUPDATE_H
