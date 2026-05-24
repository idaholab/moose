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

#include "libmesh/utility.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SystemBase;
class ElemInfo;

namespace libMesh
{
template <typename T>
class NumericVector;
}

namespace Moose
{
namespace FV
{
/// Linear finite-volume gradient schemes supported by system-owned gradient fields.
enum class LinearFVGradientSchemeType
{
  GreenGauss
};

/// Type of gradient values stored in a linear FV gradient field.
enum class LinearFVGradientFieldType
{
  Base,
  Limited
};
}
}

/**
 * Read-only handle to a cell-centered linear finite-volume gradient field stored on a system.
 */
class LinearFVGradientField
{
public:
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  LinearFVGradientField(
      const SystemBase & sys,
      const GradientContainer & components,
      Moose::FV::LinearFVGradientFieldType field_type = Moose::FV::LinearFVGradientFieldType::Base,
      Moose::FV::GradientLimiterType limiter_type = Moose::FV::GradientLimiterType::None);

  /// Access the underlying component vectors keyed by spatial direction.
  const GradientContainer & components() const { return _components; }

  /// System whose DOF map indexes this field.
  const SystemBase & system() const { return _sys; }

  /// Type of gradient values stored in this field.
  Moose::FV::LinearFVGradientFieldType fieldType() const { return _field_type; }

  /// Whether this field stores limited gradients.
  bool isLimited() const { return _field_type == Moose::FV::LinearFVGradientFieldType::Limited; }

  /// Limiter type for limited fields, or None for base fields.
  Moose::FV::GradientLimiterType limiterType() const { return _limiter_type; }

  /// Read one gradient component for a variable at an element.
  Real
  component(const ElemInfo & elem_info, unsigned int variable_number, unsigned int component) const;

  /// Read the full gradient for a variable at an element.
  RealVectorValue gradient(const ElemInfo & elem_info, unsigned int variable_number) const;

private:
  /// System whose dof map indexes this gradient field.
  const SystemBase & _sys;

  /// Component vectors keyed by spatial direction.
  const GradientContainer & _components;

  /// Type of gradient values stored in this field.
  const Moose::FV::LinearFVGradientFieldType _field_type;

  /// Limiter type for limited fields.
  const Moose::FV::GradientLimiterType _limiter_type;
};

/**
 * Shared storage and allocation logic for linear finite-volume cell gradients for
 * variables in the system attribute of this class
 */
class LinearFVGradientInterface
{
public:
  LinearFVGradientInterface(SystemBase & sys)
    : _sys(sys), _raw_gradient_field(_sys, _raw_grad_container)
  {
  }

  /**
   * Access the stored raw cell-centered gradient components.
   * @return Raw cell-centered gradient vectors keyed by spatial direction.
   */
  const std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  linearFVGradientContainer() const
  {
    return _raw_grad_container;
  }

  /// Access the default unlimited cell-centered gradient field.
  const LinearFVGradientField & linearFVGradientField() const { return _raw_gradient_field; }

  /**
   * Register a system-owned linear FV gradient field and return its field handle.
   *
   * The returned handle is intended to be cached by setup-time consumers so assembly does not need
   * string or map lookups.
   */
  LinearFVGradientField & registerFVGradient(
      unsigned int variable_number,
      Moose::FV::LinearFVGradientSchemeType scheme_type =
          Moose::FV::LinearFVGradientSchemeType::GreenGauss,
      Moose::FV::GradientLimiterType limiter_type = Moose::FV::GradientLimiterType::None);

  /**
   * Access the raw or limited cell-centered gradient field.
   * @param limiter_type The limiter type whose gradient field is being requested.
   * @return Read-only field handle backed by system-owned gradient storage.
   */
  const LinearFVGradientField &
  linearFVGradientField(const Moose::FV::GradientLimiterType limiter_type) const;

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
   * @return The requested raw or limited gradient vectors ordered by spatial direction.
   */
  const std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> &
  linearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type) const;

  /**
   * Access the limiter types requested for this system.
   * @return The set of limiter types whose limited gradients should be assembled.
   * They are only assembled for the variable(s) for which they were requested not all of them
   */
  const std::unordered_set<Moose::FV::GradientLimiterType> &
  requestedLinearFVLimitedGradientTypes() const
  {
    return _requested_limited_gradient_types;
  }

protected:
  /// Compute and store requested unlimited and limited gradients for linear FV variables.
  void computeGradients();

  /**
   * Update a registered gradient field explicitly.
   */
  void updateFVGradient(const LinearFVGradientField & field);

  /// Compute and store the unlimited gradient field with each registered scheme.
  void updateBaseGradientField();

  /// Compute unlimited gradients for variables registered to one scheme.
  /// New producers should be added here without changing cached field access by consumers.
  void
  updateBaseGradientFieldForScheme(Moose::FV::LinearFVGradientSchemeType scheme_type,
                                   const std::unordered_set<unsigned int> & gradient_variables);

  /// Compute and store a requested limited gradient field.
  void updateLimitedGradient(const Moose::FV::GradientLimiterType limiter_type);

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
  requestedLinearFVLimitedGradientVariables(const Moose::FV::GradientLimiterType limiter_type) const
  {
    return libmesh_map_find(_requested_limited_gradient_variables, limiter_type);
  }

  bool needsLinearFVGradientStorage() const;

  bool hasRegisteredFVGradient(unsigned int variable_number) const;

  void initializeContainer(
      std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> & container) const;

  void initializeLimitedGradientField(const Moose::FV::GradientLimiterType limiter_type);

  /// Reference to the system object
  SystemBase & _sys;

  /// Scratch storage for raw gradients assembled during the current compute pass.
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> _temporary_gradient;

  /// Persisted raw cell-centered gradient components keyed by spatial direction.
  std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> _raw_grad_container;

  /// Read-only field handle for the unlimited gradient storage.
  LinearFVGradientField _raw_gradient_field;

  /// Registered base-gradient schemes keyed by variable number.
  std::unordered_map<unsigned int, Moose::FV::LinearFVGradientSchemeType>
      _registered_gradient_schemes;

  /// Variable numbers keyed by the scheme that produces their base gradients.
  /// This keeps producer selection per variable while consumers keep direct field handles.
  std::unordered_map<Moose::FV::LinearFVGradientSchemeType, std::unordered_set<unsigned int>>
      _registered_gradient_scheme_variables;

  /// Set of requested limiter types for which limited gradients should be computed.
  std::unordered_set<Moose::FV::GradientLimiterType> _requested_limited_gradient_types;

  /// Variable numbers requesting limited gradients, keyed by limiter type.
  std::unordered_map<Moose::FV::GradientLimiterType, std::unordered_set<unsigned int>>
      _requested_limited_gradient_variables;

  /// Persisted limited gradient components keyed by limiter type.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>>
      _raw_limited_grad_containers;

  /// Read-only field handles for persisted limited gradient storage.
  std::unordered_map<Moose::FV::GradientLimiterType, std::unique_ptr<LinearFVGradientField>>
      _limited_gradient_fields;

  /// Scratch storage for limited gradients assembled during the current compute pass.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>>
      _temporary_limited_gradient;
};
