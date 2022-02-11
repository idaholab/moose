//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * This object adds a residual equivalent to
 *
 * \f$\int_{\Omega_C} -\epsilon \frac{p}{r} dV\f$
 *
 * for use when performing axisymmetric simulations and the \f$\epsilon \nabla p\f$ term has been
 * integrated by parts as is done for both HLLC and Kurganov-Tadmor schemes
 */
class PNSFVMomentumPressureFluxRZ : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  PNSFVMomentumPressureFluxRZ(const InputParameters & params);

  // pressure kernel
  void gatherRCData(const Elem &) override {}
  void gatherRCData(const FaceInfo &) override {}

protected:
  ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _p;
  const Moose::Functor<ADReal> & _eps;
};
