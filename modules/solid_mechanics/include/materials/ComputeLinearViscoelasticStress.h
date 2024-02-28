//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLinearElasticStress.h"
#include "LinearViscoelasticityBase.h"

/**
 * Computes the stress of a linear viscoelastic material, using total
 * small strains. The mechanical strain is decomposed into the elastic
 * strain + the creep strain, the creep strain itself resulting from
 * a spring-dashpot model.
 *
 * If you need to accomodate other sources of inelastic strains, use
 * a ComputeMultipleInelasticStress material instead, associated with a
 * LinearViscoelasticStressUpdate to represent the creep strain.
 */
class ComputeLinearViscoelasticStress : public ComputeLinearElasticStress
{
public:
  static InputParameters validParams();

  ComputeLinearViscoelasticStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpStress() override;

  ///@{ Creep strain variable
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
  ///@}

  /// Apparent creep strain (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankTwoTensor> & _apparent_creep_strain;
  /// Apparent elasticity tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  /// Instantaneous compliance tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _elasticity_tensor_inv;
};
