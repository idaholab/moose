//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeMultipleInelasticStressBase.h"
#include "IsotropicPlasticityStressUpdate.h"
#include "PowerLawCreepStressUpdate.h"

/**
 * ComputeCreepPlasticityStress computes the stress, the consistent tangent
 * operator (or an approximation to it), and a decomposition of the strain
 * into elastic and inelastic parts.  By default finite strains are assumed.
 *
 * The elastic strain is calculated by subtracting the computed inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used specifically to combine creep and plasticity.  The solve
 * happens for both materials simultaneously rather than with a staggered approach.
 */

class ComputeCreepPlasticityStress : public ComputeMultipleInelasticStressBase
{
public:
  static InputParameters validParams();

  ComputeCreepPlasticityStress(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual std::vector<MaterialName> getInelasticModelNames() override;

  /**
   * Given the _strain_increment[_qp], iterate over all of the user-specified
   * recompute materials in order to find an admissible stress (which is placed
   * into _stress[_qp]) and set of inelastic strains, as well as the tangent operator
   * (which is placed into _Jacobian_mult[_qp]).
   * @param elastic_strain_increment The elastic part of _strain_increment[_qp] after the iterative
   * process has converged
   * @param combined_inelastic_strain_increment The inelastic part of _strain_increment[_qp] after
   * the iterative process has converged.
   */
  virtual void updateQpState(RankTwoTensor & elastic_strain_increment,
                             RankTwoTensor & combined_inelastic_strain_increment) override;

  void computeStress(const RankTwoTensor & elastic_strain_increment, RankTwoTensor & stress);
  void computeTangentOperators();
  void computeInelasticStrainIncrements(Real effective_trial_stress,
                                        const RankTwoTensor & deviatoric_trial_stress);
  void finalizeConstitutiveModels();

  PowerLawCreepStressUpdate * _creep_model;
  IsotropicPlasticityStressUpdate * _plasticity_model;

  Real _effective_creep_strain_increment;
  Real _effective_plastic_strain_increment;

  std::array<RankTwoTensor, 2> _inelastic_strain_increment;
};
