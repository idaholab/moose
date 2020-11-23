//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvection.h"
#include "NSFVAdvectionBase.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class NSFVAdvectionKernel : public FVMatAdvection, protected NSFVAdvectionBase
{
public:
  static InputParameters validParams();
  NSFVAdvectionKernel(const InputParameters & params);

protected:
  /**
   * interpolation overload for the velocity
   */
  void interpolate(Moose::FV::InterpMethod m,
                   ADRealVectorValue & interp_v,
                   const ADRealVectorValue & elem_v,
                   const ADRealVectorValue & neighbor_v);

  ADReal computeQpResidual() override;

  void residualSetup() override { clearRCCoeffs(); }
  void jacobianSetup() override { clearRCCoeffs(); }

  /// The density on the FaceInfo elem
  const ADMaterialProperty<Real> & _rho_elem;

  /// The density on the FaceInfo neighbor
  const ADMaterialProperty<Real> & _rho_neighbor;

  /// The dynamic viscosity on the FaceInfo elem
  const ADMaterialProperty<Real> & _mu_elem;

  /// The dynamic viscosity on the FaceInfo neighbor
  const ADMaterialProperty<Real> & _mu_neighbor;
};

#endif
