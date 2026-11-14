//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSLinearFVFlowPhysics.h"

/**
 * Linear-FV segregated flow physics of a two-phase diffuse-interface (volume of fluid) problem,
 * solved for the reduced pressure p_rgh = p - rho g.(x - x_ref) with the PIMPLE executioner of
 * the same name.
 *
 * It differs from the single-phase parent in three ways, all of which go through the
 * ConservativeDiffuseInterfaceRhieChowMassFlux object it creates:
 * - the momentum predictor is assembled without the pressure, gravity and capillary forces; the
 *   Rhie-Chow object adds them from the reduced pressure, the density gradient and the curvature,
 *   so the parent's momentum pressure kernel is not added;
 * - the pressure equation is the volumetric continuity div(phi) = 0 in area-integrated form,
 *   with the raw face 1/A as its coefficient, because across a density jump only the volumetric
 *   flux is divergence-free;
 * - the momentum advection uses the mass flux the volume-fraction corrector transported with,
 *   rather than a face density times the volumetric flux.
 *
 * Functor names form the contract between the objects this physics creates and the coupled
 * volume-fraction physics. Published by the Rhie-Chow object and read by the kernels and
 * boundary conditions here: phiHbyA, pressure_Ainv, pressure_boundary_normal_gradient and
 * lagged_corrected_face_phi. Published by this physics with its prefix: the interface unit
 * normal, which the volume-fraction physics names through interface_normal_functor, and with
 * surface tension the interface curvature and the capillary force density. Read from the
 * volume-fraction side: vof_rho_phi_functor and volume_fraction_functor.
 */
class WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics final : public WCNSLinearFVFlowPhysics
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics(const InputParameters & parameters);

private:
  /**
   * The parent assembles the mass form, div(rho/A grad p) = div(rho H/A), with a face density in
   * both coefficients. Across the interface the mass flux is not divergence-free, the volumetric
   * flux is, so this equation is div(1/A grad p_rgh) = div(phiHbyA) with the raw face 1/A and the
   * volumetric predictor flux less the hydrostatic and capillary face forces. Both are
   * area-integrated by the Rhie-Chow object, which the divergence kernel is told through
   * pressureCorrectionFluxIsIntegrated().
   */
  MooseFunctorName pressureDiffusionCoefficientName() const override { return "pressure_Ainv"; }
  MooseFunctorName predictorFaceFluxName() const override { return "phiHbyA"; }
  /// Pressure outlets with backflow handling, and the total-pressure condition on p_rgh
  void addOutletBC() override;
  /// No momentum pressure kernel: the Rhie-Chow object adds the reduced-pressure, gravity and
  /// capillary forces to the predictor itself
  void addMomentumPressureKernels() override {}
  /// No-slip walls carry a constrained zero-normal-flux pressure condition
  void addWallsBC() override;
  /// The diffuse-interface Rhie-Chow object, with gravity, reference point and face forces
  void addRhieChowUserObjects() override;
  /// The parent's materials, the surface tension material, and the interface normal material
  void addFunctorMaterials() override;

  /// Creates the material publishing the interface curvature and capillary force density.
  void addSurfaceTensionFunctorMaterial();

  /// Two layers for the interface normal, three when the capillary force adds the curvature
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /// Whether to create the material publishing the interface unit normal from the volume fraction
  const bool _create_geometry_functors;
};
