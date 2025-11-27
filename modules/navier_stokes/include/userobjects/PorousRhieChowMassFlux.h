//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowMassFlux.h"
#include "MooseFunctor.h"

#include <unordered_map>

#include "MooseEnum.h"

/**
 * Rhie-Chow face flux provider that treats porous media by incorporating porosity
 * into the face mass fluxes and enabling manual or automatically detected pressure
 * jumps at interfaces.
 */
class PorousRhieChowMassFlux : public RhieChowMassFlux
{
public:
  static InputParameters validParams();
  PorousRhieChowMassFlux(const InputParameters & params);

  virtual void initFaceMassFlux() override;
  virtual void populateCouplingFunctors(
      const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
      const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv) override;
  virtual void computeFaceMassFlux() override;
  virtual Real getVolumetricFaceFlux(const FaceInfo & fi) const override;

protected:
  /// Simple container for user-supplied pressure jump parameters
  struct JumpOptions
  {
    Real constant = 0.0;
    Real form_factor = 0.0;
    Real bernoulli = 0.0;
  };

  /// Returns true if the face is tagged manually as an interface
  bool isManualInterfaceFace(const FaceInfo & fi) const;

  /// Returns true if either manual tagging or porosity jump detection marks the face
  bool treatAsInterface(const FaceInfo & fi, const Moose::StateArg & time) const;

  /**
   * Compute the pressure jump that must be applied at an interface face.
   * @param fi Face information
   * @param time Evaluation time/state
   * @param downwind_is_elem Returns true if the element (as opposed to the neighbor)
   *        is downstream of the flow direction
   * @return Pair of (is jump active, pressure jump value)
   */
  std::pair<bool, Real>
  computePressureJump(const FaceInfo & fi, const Moose::StateArg & time, bool & downwind_is_elem)
      const;

  /// Helper to evaluate porosity on a face argument
  Real evaluatePorosity(const Moose::FaceArg & face, const Moose::StateArg & time) const;

  /// Helper to evaluate porosity on an element
  Real evaluatePorosity(const Moose::ElemArg & elem_arg, const Moose::StateArg & time) const;

  /// Interpolate porosity to a face according to the selected method
  Real interpolateFacePorosity(const FaceInfo & fi,
                               const Moose::StateArg & time,
                               Real elem_eps,
                               Real neighbor_eps) const;

  /// Multiply the supplied value by porosity using the appropriate interpolation
  void applyPorosityWeighting(const FaceInfo & fi,
                              const Moose::StateArg & time,
                              Real elem_eps,
                              Real neighbor_eps,
                              Real elem_value,
                              Real neighbor_value,
                              Real & face_value) const;

  /// Functor describing the porosity
  const Moose::Functor<Real> & _eps;

  /// Interpolation scheme used to evaluate porosity-dependent quantities on faces
  MooseEnum _eps_face_interp_method;

  /// Whether automatic block-to-block porosity jumps should trigger interface logic
  const bool _detect_block_interfaces;

  /// Map from boundary id to user-supplied pressure jump options
  std::unordered_map<BoundaryID, JumpOptions> _interface_jump_data;
};
