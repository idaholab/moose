//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseTypes.h"

#include "libmesh/dof_object.h"
#include "libmesh/numeric_vector.h"

#include <memory>
#include <vector>

class ElemInfo;
class RhieChowMassFlux;

/**
 * Stores Green-Gauss pressure gradients formed with separate traces on pressure-jump faces.
 *
 * This provider is coordinated by RhieChowMassFlux rather than registered as the pressure
 * variable's gradient method. It therefore remains distinct from the reconstructed pressure
 * gradient used for pressure-velocity coupling.
 */
class FVPressureJumpGreenGaussGradient
{
public:
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  /// Reconstruct one gradient per pressure cell from the current pressure and jump generations.
  void reconstruct(RhieChowMassFlux & rc);

  /// Discard all cached gradients and generation metadata.
  void clear();

  /// Whether the stored gradients match the pressure and jump currently published by rc.
  bool current(const RhieChowMassFlux & rc) const;

  /// Read the one-sided cell gradient adjacent to an interface.
  RealVectorValue gradient(const RhieChowMassFlux & rc, const ElemInfo & elem_info) const;

private:
  GradientContainer _gradient;
  dof_id_type _pressure_generation = libMesh::DofObject::invalid_id;
  dof_id_type _jump_generation = libMesh::DofObject::invalid_id;
};
