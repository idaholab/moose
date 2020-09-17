//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h" // THREAD_ID
#include "FVUtils.h"

template <typename>
class MooseVariableFV;
class SubProblem;

class NSFVBase
{
public:
  static InputParameters validParams();
  NSFVBase(const InputParameters & params);

  const MooseVariableFV<Real> * pVar() const { return _p_var; }

  const MooseVariableFV<Real> * uVar() const { return _u_var; }

  const MooseVariableFV<Real> * vVar() const { return _v_var; }

  const MooseVariableFV<Real> * wVar() const { return _w_var; }

  Real mu() const { return _mu; }

  Real rho() const { return _rho; }

private:
  const SubProblem & _nsfv_subproblem;

  const THREAD_ID _nsfv_tid;

protected:
  /// pressure variable
  const MooseVariableFV<Real> * const _p_var;
  /// x-velocity
  const MooseVariableFV<Real> * const _u_var;
  /// y-velocity
  const MooseVariableFV<Real> * const _v_var;
  /// z-velocity
  const MooseVariableFV<Real> * const _w_var;

  /// The viscosity
  const Real _mu;
  /// The density
  const Real _rho;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;
};
