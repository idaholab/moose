//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "GradientLimiterType.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Shared storage and allocation logic for linear finite-volume cell gradients.
 */
class LinearFVGradientInterface
{
public:
  LinearFVGradientInterface(SystemBase & sys) : _sys(sys) {}

  /**
   * Access the stored raw cell-centered gradient components.
   * @return Raw cell-centered gradient vectors keyed by spatial direction.
   */
  const std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  linearFVGradientContainer() const
  {
    return _raw_grad_container;
  }

  /**
   * Request storage and assembly of limiter-specific cell gradients.
   * @param limiter_type The limiter whose gradient storage should be made available.
   * @param variable_number The libMesh variable number requesting the limited gradients.
   */
  void requestLinearFVLimitedGradients(const Moose::FV::GradientLimiterType limiter_type,
                                       unsigned int variable_number);

  /**
   * Access the stored raw or limited cell-centered gradient components.
   * @param limiter_type The limiter type whose gradient container is being requested.
   * @return The requested raw or limited gradient vectors keyed by spatial direction.
   */
  const std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  linearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type) const;

  /**
   * Access the limiter types requested for this system.
   * @return The set of limiter types whose limited gradients should be assembled.
   */
  const std::unordered_set<Moose::FV::GradientLimiterType> &
  requestedLinearFVLimitedGradientTypes() const
  {
    return _requested_limited_gradient_types;
  }

protected:
  /**
   * Rebuild persistent raw and temporary gradient storage after mesh/DOF changes.
   */
  void rebuildLinearFVGradientStorage();

  /**
   * Return temporary storage for gradients during gradient assembly.
   * The returned vectors are persistent scratch storage reused across calls and swapped with the
   * final gradient container before gradient assembly returns.
   */
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  temporaryLinearFVGradientContainer()
  {
    return _temporary_gradient;
  }

  /**
   * Return temporary storage for limited gradients during gradient assembly.
   * The returned vectors are persistent scratch storage reused across calls and swapped with the
   * final limited-gradient container before gradient assembly returns.
   * @param limiter_type The limiter type whose temporary storage is being accessed.
   */
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  temporaryLinearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type)
  {
    return _temporary_limited_gradient[limiter_type];
  }

  /**
   * Access the persisted limited-gradient storage for a specific limiter.
   * @param limiter_type The limiter type whose persisted storage is being accessed.
   * @return The persisted limited-gradient vectors keyed by spatial direction.
   */
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  rawLinearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type)
  {
    return _raw_limited_grad_containers[limiter_type];
  }

  /**
   * Access the variable numbers that requested limited gradients for a specific limiter.
   * @param limiter_type The limiter type whose request set is being accessed.
   * @return The set of variable numbers that requested the limiter.
   */
  const std::unordered_set<unsigned int> &
  requestedLinearFVLimitedGradientVariables(
      const Moose::FV::GradientLimiterType limiter_type) const
  {
    return _requested_limited_gradient_variables.at(limiter_type);
  }

  bool needsLinearFVGradientStorage() const;

  void initializeContainer(
      std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> & container) const;

  /// Reference to the system object
  SystemBase & _sys;

  /// Scratch storage for raw gradients assembled during the current compute pass.
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> _temporary_gradient;

  /// Persisted raw cell-centered gradient components keyed by spatial direction.
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> _raw_grad_container;

  /// Set of requested limiter types for which limited gradients should be computed.
  std::unordered_set<Moose::FV::GradientLimiterType> _requested_limited_gradient_types;

  /// Variable numbers requesting limited gradients, keyed by limiter type.
  std::unordered_map<Moose::FV::GradientLimiterType, std::unordered_set<unsigned int>>
      _requested_limited_gradient_variables;

  /// Persisted limited gradient components keyed by limiter type.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>>
      _raw_limited_grad_containers;

  /// Scratch storage for limited gradients assembled during the current compute pass.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>>
      _temporary_limited_gradient;
};
