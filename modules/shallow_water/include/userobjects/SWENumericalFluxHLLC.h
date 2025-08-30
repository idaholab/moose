//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* Licensed under LGPL 2.1

#pragma once

#include "SWENumericalFluxBase.h"

// A robust HLLC numerical flux for SWE with:
// - hydrostatic reconstruction (Audusse) applied to the input states
// - PVRS/q-corrected wave speeds (default) for sharper shocks
// - HLLE fallback when star-state degeneracy/negativity is detected
// - optional small HLLE blend near strong compression for carbuncle protection
//
// Notes:
//  - This header mirrors the interface used by SWEFVFluxDGKernel via SWENumericalFluxBase:
//      getFlux(side, ielem, ineig, UL, UR, n)
//      getJacobian(which, side, ielem, ineig, UL, UR, n)
//  - UL/UR are [h, hu, hv, b] at the face (cell-centered reconstruction already applied upstream)
//  - n is the face unit normal (nx, ny)
//  - We return cached references (recomputed each call) for efficiency.
//
// If your local base interface differs slightly, adapt only the signature glue;
// the core algorithm is implemented in computeFlux_().

class SWENumericalFluxHLLC : public SWENumericalFluxBase
{
public:
  static InputParameters validParams();
  SWENumericalFluxHLLC(const InputParameters & parameters);
  virtual ~SWENumericalFluxHLLC();

  /**
   * Solve the Riemann problem
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   * @param[out]  flux      flux vector across the side
   */
  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  /**
   * Compute the Jacobian matrix
   * @param[in]   iside     local  index of current side
   * @param[in]   ielem     global index of the current element
   * @param[in]   ineig     global index of the neighbor element
   * @param[in]   uvec1     vector of variables on the "left"
   * @param[in]   uvec2     vector of variables on the "right"
   * @param[in]   dwave     vector of unit normal
   * @param[out]  jac1      Jacobian matrix contribution to the "left" cell
   * @param[out]  jac2      Jacobian matrix contribution to the "right" cell
   */
  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const override;

  // Optional debug accounting
  virtual void initialize() override;
  virtual void finalize() override;

private:
  // Tunables
  const bool _use_pvrs;       // PVRS/q wave speeds if true, else Einfeldt
  const Real _degeneracy_eps; // clamp for S* and SR-SL; also positivity threshold
  const Real _blend_alpha;    // 0..~0.5; 0 = pure HLLC

  // Debug controls
  const bool _log_debug;                 // log counters each timestep
  mutable unsigned long _fallback_count; // how many faces used HLLE fallback this step
  mutable unsigned long _blend_count;    // how many faces applied blending this step

  // mutable caches (recomputed each call)
  mutable std::vector<Real> _flux;
  mutable DenseMatrix<Real> _dF_dUL;
  mutable DenseMatrix<Real> _dF_dUR;
};
