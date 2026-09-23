//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "PorousViscoplasticityStressUpdate.h"
#include "MooseEnum.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * Shared test-only extension of the generic porous-LPS material.
 *
 * The extension supplies two independently evolving pore populations with prescribed linear
 * pressure closures and explicit population floors. This keeps tests of the generic independent
 * local solver in solid_mechanics without depending on BISON gas EOS, topology, or inventory.
 */
template <bool is_ad>
class PorousViscoplasticityStressUpdateTestStateTempl
  : public PorousViscoplasticityStressUpdateTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousViscoplasticityStressUpdateTestStateTempl(const InputParameters & parameters);

protected:
  using Base = PorousViscoplasticityStressUpdateTempl<is_ad>;
  using PorePorosityState = typename Base::PorePorosityState;
  using HydrostaticStressState = typename Base::HydrostaticStressState;

  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void resetIncrementalMaterialProperties() override;

  virtual bool useIndependentPorePorosityKinematics() const override
  {
    return _use_prescribed_two_population_state;
  }

  virtual PorePorosityState
  independentPorePorosityState(const GenericReal<is_ad> & total_porosity) const override;

  virtual GenericReal<is_ad>
  independentPorePorosityFloor(unsigned int population_index,
                               const PorePorosityState & pore_porosity_begin) const override;

  virtual HydrostaticStressState
  evaluateIndependentHydrostaticStress(const GenericReal<is_ad> & matrix_hydro_stress,
                                       const PorePorosityState & pore_porosity) const override;

  virtual void independentPorePorosityStateAccepted(
      const GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const PorePorosityState & pore_porosity) override;

  const bool _use_prescribed_two_population_state;
  const Real _initial_population_0_fraction;
  const Real _test_initial_total_porosity;
  const std::vector<Real> _population_pressures;
  const std::vector<Real> _population_pressure_derivatives;
  const std::vector<Real> _population_porosity_floors;

  GenericMaterialProperty<Real, is_ad> & _test_population_0_porosity;
  const MaterialProperty<Real> & _test_population_0_porosity_old;
  GenericMaterialProperty<Real, is_ad> & _test_population_1_porosity;
  const MaterialProperty<Real> & _test_population_1_porosity_old;
  GenericMaterialProperty<Real, is_ad> & _test_population_0_effective_hydrostatic_stress;
  GenericMaterialProperty<Real, is_ad> & _test_population_1_effective_hydrostatic_stress;
};

/**
 * Test-only non-AD material that audits compiled porous-LPS kernels and supplies prescribed
 * two-population mechanics for generic local-solver regressions.
 */
class PorousViscoplasticityStressUpdateTest
  : public PorousViscoplasticityStressUpdateTestStateTempl<false>
{
public:
  static InputParameters validParams();

  PorousViscoplasticityStressUpdateTest(const InputParameters & parameters);

  virtual void updateStateSubstep(RankTwoTensor & strain_increment,
                                  RankTwoTensor & inelastic_strain_increment,
                                  const RankTwoTensor & rotation_increment,
                                  RankTwoTensor & stress_new,
                                  const RankTwoTensor & stress_old,
                                  const RankFourTensor & elasticity_tensor,
                                  const RankTwoTensor & elastic_strain_old,
                                  bool compute_full_tangent_operator,
                                  RankFourTensor & tangent_operator) override;

protected:
  using Base = PorousViscoplasticityStressUpdateTestStateTempl<false>;
  using PorePorosityState = typename Base::PorePorosityState;

  virtual void initQpStatefulProperties() override;
  virtual Real scalarPorosityFloor() const override;
  virtual unsigned int estimateNumberSubsteps(const RankTwoTensor & stress) override;
  virtual PorePorosityState independentPorePorosityState(const Real & total_porosity) const override;
  virtual void independentPorePorosityStateAccepted(
      const RankTwoTensor & inelastic_strain_increment,
      const PorePorosityState & pore_porosity) override;

private:
  struct RollbackState
  {
    RankTwoTensor strain_increment;
    RankTwoTensor inelastic_strain_increment;
    RankTwoTensor stress;
    RankFourTensor tangent;

    Real intermediate_porosity = 0.0;
    Real effective_inelastic_strain = 0.0;
    RankTwoTensor inelastic_strain;
    Real effective_inelastic_strain_rate = 0.0;
    Real substep_control_inelastic_strain_rate = 0.0;
    Real hydro_stress = 0.0;
    Real gauge_stress = 0.0;
    std::vector<Real> gauge_stresses;
    Real population_0_porosity = 0.0;
    Real population_1_porosity = 0.0;
    std::uint64_t constitutive_retries = 0;
  };

  RollbackState captureRollbackState(const RankTwoTensor & strain_increment,
                                     const RankTwoTensor & inelastic_strain_increment,
                                     const RankTwoTensor & stress,
                                     const RankFourTensor & tangent) const;
  void verifyRollbackState(const RollbackState & expected,
                           const RankTwoTensor & strain_increment,
                           const RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & stress,
                           const RankFourTensor & tangent) const;
  void throwInjectedFailure(const char * stage) const;
  std::string failurePointName() const;

  void runKernelChecks();
  void checkN1Derivatives(Real pressure, Real deffective_hydro_df) const;
  void checkHigherPowerNearZeroPressure(Real power) const;
  void checkHAndCreepKernels() const;
  void checkGaugeResidualKernels();
  void checkIndependentDerivativeKernels() const;
  void checkIndependentProjectionKernels() const;

  void checkClose(const char * label,
                  Real actual,
                  Real expected,
                  Real relative_tolerance = 5.0e-13,
                  Real absolute_tolerance = 5.0e-30) const;

  const bool _run_kernel_checks;
  const MooseEnum _failure_point;
  bool _kernel_checks_complete = false;
  bool _inside_update_state_substep = false;
  unsigned int _accepted_state_calls = 0;
  unsigned int _accepted_path_substeps = 0;
};

/** AD counterpart used to exercise the generic independent two-population solve. */
class ADPorousViscoplasticityStressUpdateTest
  : public PorousViscoplasticityStressUpdateTestStateTempl<true>
{
public:
  static InputParameters validParams();

  ADPorousViscoplasticityStressUpdateTest(const InputParameters & parameters);

protected:
  using Base = PorousViscoplasticityStressUpdateTestStateTempl<true>;
};
