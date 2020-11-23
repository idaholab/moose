//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseConfig.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "MooseTypes.h" // THREAD_ID
#include "FVUtils.h"

#include <unordered_map>
#include <vector>

template <typename>
class MooseVariableFV;
class SubProblem;
class MooseApp;

/**
 * Common base that both NSFV flux kernels and flux boundary conditions can inherit from
 */
class NSFVAdvectionBase
{
protected:
  static InputParameters validParams();
  NSFVAdvectionBase(const InputParameters & params);

  /**
   * pressure variable getter
   */
  const MooseVariableFV<Real> * pVar() const { return _p_var; }

  /**
   * x-component of velocity. May also be radial *or* axial velocity in RZ coordinates
   */
  const MooseVariableFV<Real> * uVar() const { return _u_var; }

  /**
   * y-component of velocity. May also be radial *or* axial velocity in RZ coordinates
   */
  const MooseVariableFV<Real> * vVar() const { return _v_var; }

  /**
   * z-component of velocity
   */
  const MooseVariableFV<Real> * wVar() const { return _w_var; }

  /**
   * Returns the Rhie-Chow 'a' coefficient for the requested element \p elem
   * @param elem The elem to get the Rhie-Chow coefficient for
   * @param mu The dynamic viscosity
   * @param rho The density
   */
  const ADReal & rcCoeff(const Elem & elem, const ADReal & mu, const ADReal & rho) const;

  /**
   * Clear the RC 'a' coefficient cache
   */
  void clearRCCoeffs();

private:
  const MooseApp & _nsfv_app;
  const SubProblem & _nsfv_subproblem;
  const THREAD_ID _nsfv_tid;

  /// A map from elements to the 'a' coefficients used in the Rhie-Chow interpolation. The size of
  /// the vector is equal to the number of threads in the simulation. We maintain a map from
  /// MooseApp pointer to RC coefficients in order to support MultiApp simulations
  static std::unordered_map<const MooseApp *, std::vector<std::unordered_map<const Elem *, ADReal>>>
      _rc_a_coeffs;

  /**
   * method for computing the Rhie-Chow 'a' coefficients for the given elem \p elem
   * @param elem The elem to compute the Rhie-Chow coefficient for
   * @param mu The dynamic viscosity
   * @param rho The density
   */
  ADReal coeffCalculator(const Elem & elem, const ADReal & mu, const ADReal & rho) const;

protected:
  /// pressure variable
  const MooseVariableFV<Real> * const _p_var;
  /// x-velocity
  const MooseVariableFV<Real> * const _u_var;
  /// y-velocity
  const MooseVariableFV<Real> * const _v_var;
  /// z-velocity
  const MooseVariableFV<Real> * const _w_var;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;
};

#endif
