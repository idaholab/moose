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
 * Sharp-interface Rhie-Chow implementation for the reference-parity momentum path.
 *
 * The momentum unknowns are velocity components U_i. Conservative behavior is supplied by the
 * density-weighted matrix coefficients and face fluxes, matching the interFoam style
 * ddt(rho, U) + div(rhoPhi, U) form without storing rho*U as a primary unknown.
 */
class ConservativeSharpInterfaceRhieChowMassFlux : public ConservativeSharpInterfaceRhieChowMassFluxBase
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfaceRhieChowMassFlux(const InputParameters & params);

  void updateContinuityErrorField();
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) const override;

  bool debugUsingCachedPredictorOperator() const;
  Real debugCurrentVelocityComponent(const ElemInfo & elem_info, const unsigned int component) const;
  Real debugLastWritebackPreVelocityComponent(const ElemInfo & elem_info,
                                              const unsigned int component) const;
  Real debugLastWritebackPostVelocityComponent(const ElemInfo & elem_info,
                                               const unsigned int component) const;
  Real debugLastWritebackPressureDeltaVelocityComponent(const ElemInfo & elem_info,
                                                        const unsigned int component) const;
  Real debugLivePredictorBaseRawComponent(const ElemInfo & elem_info,
                                          const unsigned int component) const;
  Real debugCachedPredictorBaseRawComponent(const ElemInfo & elem_info,
                                            const unsigned int component) const;
  Real debugVelocityPredictorBaseRawComponent(const ElemInfo & elem_info,
                                              const unsigned int component) const;
  Real debugVelocityPredictorHbyAComponent(const ElemInfo & elem_info,
                                           const unsigned int component) const;

private:
  void computePredictorOperatorBaseForSolution(const unsigned int system_i,
                                               const NumericVector<Number> & solution_override,
                                               NumericVector<Number> & base_raw,
                                               NumericVector<Number> & diagonal_raw) const;
  void buildVelocityPredictorState(const unsigned int system_i,
                                   const bool with_updated_pressure,
                                   NumericVector<Number> & hbya_raw,
                                   NumericVector<Number> & ainv_raw) const;

  CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _continuity_error;
};
