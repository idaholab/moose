/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RETURNMAPPINGMODEL_H
#define RETURNMAPPINGMODEL_H

#include "ConstitutiveModel.h"
#include "SingleVariableReturnMappingSolution.h"

/**
 * Base class for models that perform return mapping iterations to compute stress.  This
 * is for a single model to compute inelastic behavior.  This class can be used directly
 * as a standalone model for a single source of inelasticity.  It can also be called from
 * another model that combines together multiple inelastic models using Picard iterations.
 */

class ReturnMappingModel : public ConstitutiveModel, public SingleVariableReturnMappingSolution
{
public:
  ReturnMappingModel(const InputParameters & parameters,
                     const std::string inelastic_strain_name = "");
  virtual ~ReturnMappingModel() {}

  virtual void initQpStatefulProperties() override;

  virtual void computeStress(const Elem & current_elem,
                             const SymmElasticityTensor & elasticityTensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new) override;

  /**
   * Compute stress by performing return mapping iterations.  This can be called either
   * from within this model, or by an external model that combines multiple inelastic models.
   * @param current_elem               Current element
   * @param elasticityTensor           Elasticity tensor
   * @param stress_old                 Old state of stress
   * @param strain_increment           Strain increment
   * @param stress_new                 New state of stress
   * @param inelastic_strain_increment Inelastic strain increment
   */
  void computeStress(const Elem & current_elem,
                     const SymmElasticityTensor & elasticityTensor,
                     const SymmTensor & stress_old,
                     SymmTensor & strain_increment,
                     SymmTensor & stress_new,
                     SymmTensor & inelastic_strain_increment);

  virtual Real computeReferenceResidual(const Real effective_trial_stress,
                                        const Real scalar) override;

  /**
   * Compute the limiting value of the time step for this material
   * @return Limiting time step
   */
  Real computeTimeStepLimit();

protected:
  /**
   * Perform any necessary initialization before return mapping iterations
   * @param effectiveTrialStress Effective trial stress
   * @param elasticityTensor     Elasticity tensor
   */
  virtual void computeStressInitialize(Real /*effectiveTrialStress*/,
                                       const SymmElasticityTensor & /*elasticityTensor*/)
  {
  }

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void computeStressFinalize(const SymmTensor & /*inelasticStrainIncrement*/) {}

  Real _effective_strain_increment;

  /// 3 * shear modulus
  Real _three_shear_modulus;

  MaterialProperty<Real> & _effective_inelastic_strain;
  const MaterialProperty<Real> & _effective_inelastic_strain_old;
  Real _max_inelastic_increment;
};

template <>
InputParameters validParams<ReturnMappingModel>();

#endif // RETURNMAPPINGMODEL_H
