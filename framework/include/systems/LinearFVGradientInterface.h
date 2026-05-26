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
class FVGradientMethod;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Read-only handle to a cell-centered linear finite-volume gradient field stored on a system.
 */
class LinearFVGradientField
{
public:
  /// One vector per spatial component of the cell-centered gradient.
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  /**
   * @param sys System that owns the variables and the gradient field storage.
   * @param components Component vectors that store the gradient values.
   * @param method Gradient method that produces the values in this field.
   */
  LinearFVGradientField(const SystemBase & sys,
                        const GradientContainer & components,
                        const FVGradientMethod & method);

  /// Access the underlying component vectors keyed by spatial direction.
  const GradientContainer & components() const { return _components; }

  /// System whose DOF map indexes this field.
  const SystemBase & system() const { return _sys; }

  /// Method object that produces this field.
  const FVGradientMethod & method() const { return _method; }

  /// Whether this field stores limited gradients.
  bool isLimited() const { return limiterType() != Moose::FV::GradientLimiterType::None; }

  /// Limiter type for limited fields, or None for unlimited fields.
  Moose::FV::GradientLimiterType limiterType() const;

  /**
   * Read one gradient component for a variable at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param variable_number Variable number whose gradient should be read.
   * @param component Spatial component of the gradient.
   */
  Real
  component(const ElemInfo & elem_info, unsigned int variable_number, unsigned int component) const;

  /**
   * Read the full gradient for a variable at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param variable_number Variable number whose gradient should be read.
   */
  RealVectorValue gradient(const ElemInfo & elem_info, unsigned int variable_number) const;

private:
  /// System whose dof map indexes this gradient field.
  const SystemBase & _sys;

  /// Component vectors keyed by spatial direction.
  const GradientContainer & _components;

  /// Method object that produces this field.
  const FVGradientMethod & _method;
};

/**
 * Shared registration, storage, update, and allocation logic for system-owned linear
 * finite-volume cell gradients.
 */
class LinearFVGradientInterface
{
public:
  /**
   * @param sys System that owns registered linear finite-volume gradient fields.
   */
  LinearFVGradientInterface(SystemBase & sys) : _sys(sys) {}

  /**
   * Register a system-owned linear FV gradient field produced by a method object.
   * @param variable_number Variable number whose gradient should be stored in the field.
   * @param method Gradient method that computes the field values.
   */
  LinearFVGradientField & registerFVGradient(unsigned int variable_number,
                                             const FVGradientMethod & method);

protected:
  /// Update all registered linear FV gradient fields.
  void computeGradients();

  /**
   * Update a registered gradient field explicitly.
   * @param field Gradient field to update.
   */
  void updateFVGradient(const LinearFVGradientField & field);

  /**
   * Rebuild persistent and temporary gradient storage after mesh/DOF changes.
   */
  void rebuildLinearFVGradientStorage();

  /// Whether any linear finite-volume gradient fields have been registered on this system.
  bool needsLinearFVGradientStorage() const;

  /**
   * Allocate one zeroed vector per spatial component for gradient storage.
   * @param container Component-vector container to rebuild.
   */
  void initializeContainer(
      std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> & container) const;

  /// One vector per spatial component of a cell-centered gradient field.
  using GradientContainer = LinearFVGradientField::GradientContainer;

  /// Storage owned by the system for all variables using the same gradient method.
  struct LinearFVGradientFieldStorage
  {
    /**
     * @param method Gradient method that produces this storage's field values.
     */
    LinearFVGradientFieldStorage(const FVGradientMethod & method) : method(method) {}

    /// Gradient method that produces this storage's field values.
    const FVGradientMethod & method;

    /// Variable numbers whose gradients are stored in this field.
    std::unordered_set<unsigned int> variable_numbers;

    /// Persistent gradient values read by consumers through field.
    GradientContainer values;

    /// Scratch space where the method writes final values before swapping into values.
    GradientContainer output_scratch;

    /// Scratch space the method can use for pre-limiter values or composed method calls.
    GradientContainer method_scratch;

    /// Field handle returned to consumers and backed by values.
    std::unique_ptr<LinearFVGradientField> field;
  };

  /**
   * Find or create the storage associated with a gradient method.
   * @param method Gradient method whose storage should be used.
   */
  LinearFVGradientFieldStorage & methodGradientStorage(const FVGradientMethod & method);

  /**
   * Allocate persistent and scratch vectors for a registered gradient method.
   * @param storage Method storage whose vectors should be rebuilt.
   */
  void initializeMethodGradientStorage(LinearFVGradientFieldStorage & storage);

  /**
   * Recompute the field values for a registered gradient method.
   * @param storage Method storage whose values should be updated.
   */
  void updateMethodGradientStorage(LinearFVGradientFieldStorage & storage);

  /// Reference to the system object
  SystemBase & _sys;

  /// Gradient field storages keyed by the method object that produces them.
  std::unordered_map<const FVGradientMethod *, std::unique_ptr<LinearFVGradientFieldStorage>>
      _registered_gradient_method_fields;
};
