//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConservativeSharpInterfaceRhieChowMassFluxBase.h"
#include "CellCenteredMapFunctor.h"

/**
 * Parallel sharp-interface Rhie-Chow implementation for conservative momentum systems whose
 * primary unknowns are rho*u_i rather than u_i.
 *
 * This class owns the conservative pressure-correction/writeback path while reusing the shared
 * sharp-interface face-flux infrastructure from
 * ConservativeSharpInterfaceRhieChowMassFluxBase. Reduced-pressure executioners explicitly
 * detect this user object and call computeConservativeHbyA() instead of the velocity-form
 * computeHbyA() path.
 */
class ConservativeSharpInterfaceRhieChowMassFlux : public ConservativeSharpInterfaceRhieChowMassFluxBase
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfaceRhieChowMassFlux(const InputParameters & params);

  void initFaceMassFlux() override;
  void computeCellVelocity() override;
  void updateAdditionalPressureFluxFunctors(const bool with_updated_pressure,
                                            const bool verbose) override;
  void updateContinuityErrorField();

  /// Conservative equivalent of computeHbyA() for rho*u systems.
  void computeConservativeHbyA(const bool with_updated_pressure, const bool verbose);

  bool debugUsingCachedPredictorOperator() const;
  Real debugCurrentMomentumComponent(const ElemInfo & elem_info, const unsigned int component) const;
  Real debugCurrentVelocityComponent(const ElemInfo & elem_info, const unsigned int component) const;
  Real debugLastWritebackPreMomentumComponent(const ElemInfo & elem_info,
                                              const unsigned int component) const;
  Real debugLastWritebackPostMomentumComponent(const ElemInfo & elem_info,
                                               const unsigned int component) const;
  Real debugLastWritebackPressureDeltaVelocityComponent(const ElemInfo & elem_info,
                                                        const unsigned int component) const;
  Real debugLastWritebackPressureDeltaMomentumComponent(const ElemInfo & elem_info,
                                                        const unsigned int component) const;
  Real debugLivePredictorBaseRawComponent(const ElemInfo & elem_info,
                                          const unsigned int component) const;
  Real debugCachedPredictorBaseRawComponent(const ElemInfo & elem_info,
                                            const unsigned int component) const;
  Real debugDerivedVelocityPredictorBaseRawComponent(const ElemInfo & elem_info,
                                                     const unsigned int component) const;
  Real debugDerivedVelocityPredictorHbyAComponent(const ElemInfo & elem_info,
                                                  const unsigned int component) const;

protected:
  Real cellPhysicalVelocityComponent(const ElemInfo & elem_info,
                                     const unsigned int component,
                                     const Moose::StateArg & time_arg) const override;
  Real boundaryPhysicalVelocityComponent(const FaceInfo * fi,
                                         const unsigned int component,
                                         const Moose::StateArg & time_arg) const override;

private:
  void rebuildAuthoritativeVelocitySolutionFromMomentum();
  void clearAuthoritativeVelocitySolution();
  void computePredictorOperatorBaseForSolution(const unsigned int system_i,
                                               const NumericVector<Number> & solution_override,
                                               NumericVector<Number> & base_raw,
                                               NumericVector<Number> & diagonal_raw) const;
  std::unique_ptr<NumericVector<Number>>
  buildDerivedVelocitySolution(const unsigned int system_i) const;
  void buildVelocityProjectionDensityVector(const unsigned int system_i,
                                            NumericVector<Number> & density_raw) const;
  void convertConservativePredictorStateToVelocityForm(const unsigned int system_i,
                                                       const bool with_updated_pressure,
                                                       NumericVector<Number> & hbya_raw,
                                                       NumericVector<Number> & ainv_raw) const;
  void buildDerivedVelocityPredictorState(const unsigned int system_i,
                                          const bool with_updated_pressure,
                                          NumericVector<Number> & hbya_raw,
                                          NumericVector<Number> & ainv_raw) const;
  void populateConservativeCouplingFunctors(
      const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
      const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv);
  void writeProvisionalMomentumToMomentumSolution(const Moose::StateArg & time_arg);
  Real boundaryMomentumComponentValue(const FaceInfo * fi,
                                      const unsigned int component,
                                      const Moose::StateArg & time_arg) const;

  std::vector<std::unordered_map<dof_id_type, Real>> _last_writeback_pre_momentum;
  std::vector<std::unordered_map<dof_id_type, Real>> _last_writeback_post_momentum;
  std::vector<std::unordered_map<dof_id_type, Real>> _last_writeback_pressure_delta_velocity;
  std::vector<std::unique_ptr<NumericVector<Number>>> _authoritative_velocity_solution_raw;
  bool _authoritative_velocity_solution_valid = false;
  const bool _use_face_based_reduced_pressure_predictor_contract;
  CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _continuity_error;
};
