//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "MooseEnum.h"

/**
 * MeshGenerator for doing mesh smoothing
 */
class SmoothMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SmoothMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Smoothing algorithm to use
  const MooseEnum _algorithm;

  /// Laplace only: the number of smoothing passes to do
  const unsigned int _iterations;

  /// Variational only: the dilation weight (variational smoother only)
  const Real _dilation_weight;

  /// Variational only: whether to preserve subdomain/block boundaries during smoothing
  const bool _preserve_subdomain_boundaries;

  /// Variational only: solver relative residual tolerance
  const Real _relative_residual_tolerance;

  /// Variational only: solver absolute residual tolerance
  const Real _absolute_residual_tolerance;

  /// Variational only: verbosity level between 0 and 100
  const unsigned int _verbosity;
};
