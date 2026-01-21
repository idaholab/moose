//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "FVInterpolationMethodInterface.h"
#include "FVInterpolationMethod.h"
#include <unordered_map>

/**
 * Kernel that adds contributions from an advection term discretized using the finite volume method
 * to a linear system.
 */
class LinearFVAdvection : public LinearFVFluxKernel, public FVInterpolationMethodInterface
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVAdvection(const InputParameters & params);

  virtual void setupFaceData(const FaceInfo * face_info) override;
  virtual void initialSetup() override;

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

protected:
  /// Constant advecting velocity vector
  const RealVectorValue _velocity;

  /// The interpolation method to use for the advected quantity
  const FVInterpolationMethod * _adv_interp_method;

  /// Cached handle used to evaluate the advected interpolation method without virtual dispatch
  FVInterpolationMethod::AdvectedSystemContributionCalculator _adv_interp_handle;

  /// Cached weights/correction for the current face (refreshed in setupFaceData)
  mutable FVInterpolationMethod::AdvectedSystemContribution _adv_interp_result;

  Real _adv_face_flux = 0.0;
};
