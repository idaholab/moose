
///* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAuxKernel.h"

/**
 * Vector coefficient computing the meridional curl of an
 * azimuthal scalar field A_theta.
 *
 * For a 2D meridional mesh with coordinates
 *
 *   x = r
 *   y = z
 *
 * this computes
 *
 *   B_r = -dA_theta/dz
 *   B_z =  dA_theta/dr + A_theta/r
 */
class MeridionalCurlVectorCoefficient : public mfem::VectorCoefficient
{
public:
  MeridionalCurlVectorCoefficient(mfem::Coefficient & a_theta,
                                  mfem::VectorCoefficient & grad_a_theta);

  using mfem::VectorCoefficient::Eval;

  void Eval(mfem::Vector & V,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;

private:
  mfem::Coefficient & _a_theta;
  mfem::VectorCoefficient & _grad_a_theta;
};

/**
 * Auxiliary kernel computing the meridional curl associated
 * with an azimuthal scalar field A_theta.
 */
class MFEMMeridionalCurlAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMMeridionalCurlAux(const InputParameters & parameters);

  virtual ~MFEMMeridionalCurlAux() = default;

  /// Computes the auxiliary variable.
  virtual void execute() override;

protected:
  /// Custom coefficient implementing meridional curl.
  MeridionalCurlVectorCoefficient _curl_coef;
};

#endif
