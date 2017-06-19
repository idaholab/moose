/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECOMPUTERADIALRETURN_H
#define RECOMPUTERADIALRETURN_H

#include "StressUpdateBase.h"

// Forward declaration
class RadialReturnStressUpdate;

template <>
InputParameters validParams<RadialReturnStressUpdate>();

/**
 * RadialReturnStressUpdate computes the radial return stress increment for
 * an isotropic viscoplasticity plasticity model after interating on the difference
 * between new and old trial stress increments.  This radial return mapping class
 * acts as a base class for the radial return creep and plasticity classes / combinations.
 * The stress increment computed by RadialReturnStressUpdate is used by
 * ComputeRadialReturnMappingStress which computes the elastic stress for finite
 * strains.  This return mapping class is acceptable for finite strains but not
 * total strains.
 * This class is based on the Elasto-viscoplasticity algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University Press.
 */

class RadialReturnStressUpdate : public StressUpdateBase
{
public:
  RadialReturnStressUpdate(const InputParameters & parameters,
                           const std::string inelastic_strain_name = "");

  /// A radial return (J2) mapping method is defined in this material by overwritting
  /// the computeInelasticStrainIncrement method.
  virtual void updateState(RankTwoTensor & strain_increment,
                           RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & rotation_increment,
                           RankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const RankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;
  virtual Real computeTimeStepLimit() override;

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return true; }

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeStressInitialize(Real /*effectiveTrialStress*/,
                                       const RankFourTensor & /*elasticity_tensor*/)
  {
  }
  virtual void iterationInitialize(Real /*scalar*/) {}
  virtual Real computeResidual(Real /*effectiveTrialStress*/, Real /*scalar*/) { return 0; }
  virtual Real computeDerivative(Real /*effectiveTrialStress*/, Real /*scalar*/) { return 0; }
  virtual void iterationFinalize(Real /*scalar*/) {}
  virtual void computeStressFinalize(const RankTwoTensor & /*inelasticStrainIncrement*/) {}

  const unsigned int _max_its;
  const bool _output_iteration_info;
  const bool _output_iteration_info_on_error;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;

  MaterialProperty<Real> & _effective_inelastic_strain;
  MaterialProperty<Real> & _effective_inelastic_strain_old;
  Real _max_inelastic_increment;
};

#endif // RECOMPUTERADIALRETURN_H
