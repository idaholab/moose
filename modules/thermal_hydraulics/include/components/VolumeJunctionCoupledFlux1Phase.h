//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Applies a flux between a VolumeJunction1Phase component and an external application.
 */
class VolumeJunctionCoupledFlux1Phase : public Component
{
public:
  static InputParameters validParams();

  VolumeJunctionCoupledFlux1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /**
   * Adds a VolumeJunctionCoupledFlux1PhaseKernel
   *
   * @param[in] var  Volume junction variable on which kernel acts
   * @param[in] i  Equation index within volume junction equations
   */
  void addVolumeJunctionKernel(const std::string & var, unsigned int i);

  /**
   * Adds a VolumeJunctionCoupledFlux1PhasePostprocessor
   *
   * @param[in] equation  Equation for which to get flux: "mass" or "energy"
   */
  void addFluxPostprocessor(const std::string & equation);

  /**
   * Adds a MultiAppPostprocessorTransfer for a flux PP
   *
   * @param[in] equation  Equation for which to get flux: "mass" or "energy"
   */
  void addFluxTransfer(const std::string & equation);

  /**
   * Adds a Receiver post-processor
   *
   * @param[in] property  Property to receive: "p" or "T"
   */
  void addPropertyPostprocessor(const std::string & property);

  /**
   * Adds a MultiAppPostprocessorTransfer to get a property
   *
   * @param[in] property  Property to receive: "p" or "T"
   */
  void addPropertyTransfer(const std::string & property);

  /**
   * Returns the input with the post-processor suffix
   *
   * @param[in] property  String to suffix
   */
  PostprocessorName addPostprocessorSuffix(const std::string & base_name) const;

  /// Volume junction name
  const std::string & _volume_junction_name;
  /// Unnormalized normal vector from junction
  const RealVectorValue & _normal_from_junction_unnormalized;
  /// Normalized normal vector from junction
  const RealVectorValue _normal_from_junction;
  /// Suffix to append to post-processor names
  const std::string & _pp_suffix;
};
