//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMVectorMassKernel.h"

/**
 * \f[
 * (k d\vec u/dt, \vec v)
 * \f]
 * for \f$\vec u\f$ and \f$\vec v\f$ in vector-valued \f$H^1\f$ or \f$L^2\f$ spaces.
 */
class MFEMTimeDerivativeVectorMassKernel : public MFEMVectorMassKernel
{
public:
  static InputParameters validParams();

  MFEMTimeDerivativeVectorMassKernel(const InputParameters & parameters);

  /// Get name of the time derivative of the trial variable the kernel acts on.
  virtual const VariableName & getTrialVariableName() const override { return _var_dot_name; }

protected:
  /// Name of variable (gridfunction) representing time derivative of the trial variable.
  const VariableName _var_dot_name;
};

#endif
