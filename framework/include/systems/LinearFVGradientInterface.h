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
class FaceInfo;
class FVGradientMethod;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Read-only view of one variable's cell-centered linear finite-volume gradient values.
 */
class LinearFVGradientReader
{
public:
  /// One vector per spatial component of the cell-centered gradient.
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  /**
   * @param sys System that owns the variables and gradient values.
   * @param components Component vectors that store the gradient values.
   * @param method Gradient method that produces the values read by this object.
   * @param variable_number Variable number whose gradient this object reads.
   */
  LinearFVGradientReader(const SystemBase & sys,
                         const GradientContainer & components,
                         const FVGradientMethod & method,
                         unsigned int variable_number);

  /// Access the underlying component vectors keyed by spatial direction.
  const GradientContainer & components() const { return _components; }

  /// System whose DOF map indexes the stored values.
  const SystemBase & system() const { return _sys; }

  /// Method object that produces the stored values.
  const FVGradientMethod & method() const { return _method; }

  /// Variable number whose gradients are read by this object.
  unsigned int variableNumber() const { return _variable_number; }

  /// Whether this reader accesses limited gradients.
  bool isLimited() const { return limiterType() != Moose::FV::GradientLimiterType::None; }

  /// Limiter type for limited fields, or None for unlimited fields.
  Moose::FV::GradientLimiterType limiterType() const;

  /**
   * Read one gradient component at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param component Spatial component of the gradient.
   */
  Real component(const ElemInfo & elem_info, unsigned int component) const;

  /**
   * Read the full gradient at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   */
  RealVectorValue gradient(const ElemInfo & elem_info) const;

  /**
   * Read the full gradient interpolated to a face.
   * @param fi Face whose interpolated gradient should be read.
   */
  RealVectorValue gradient(const FaceInfo & fi) const;

private:
  /// System whose dof map indexes the stored values.
  const SystemBase & _sys;

  /// System number cached for hot DOF lookups.
  const unsigned int _system_number;

  /// Component vectors keyed by spatial direction.
  const GradientContainer & _components;

  /// Method object that produces the stored values.
  const FVGradientMethod & _method;

  /// Variable number whose gradients are read by this object.
  const unsigned int _variable_number;
};

/**
 * Shared registration, update, and allocation logic for system-owned linear finite-volume cell
 * gradients.
 */
class LinearFVGradientInterface
{
public:
  /**
   * @param sys System that owns registered linear finite-volume gradient fields.
   */
  LinearFVGradientInterface(SystemBase & sys) : _sys(sys) {}

  /**
   * Register a variable for system-owned linear FV gradient values produced by a method object.
   * @param variable_number Variable number whose gradient should be stored.
   * @param method Gradient method that computes the field values.
   */
  LinearFVGradientReader registerFVGradient(unsigned int variable_number,
                                            const FVGradientMethod & method);

protected:
  /// One vector per spatial component of a cell-centered gradient field.
  using GradientContainer = LinearFVGradientReader::GradientContainer;

  /// Update all registered linear FV gradient fields.
  void computeGradients();

  /**
   * Update a registered gradient reader explicitly.
   * @param reader Gradient reader to update.
   */
  void updateFVGradient(const LinearFVGradientReader & reader);

  /**
   * Rebuild cached gradient values and reusable scratch storage after mesh/DOF changes.
   */
  void rebuildLinearFVGradientStorage();

  /// Whether any linear finite-volume gradient fields have been registered on this system.
  bool needsLinearFVGradientStorage() const;

  /**
   * Allocate one zeroed vector per spatial component for gradient storage.
   * @param container Component-vector container to rebuild.
   */
  void initializeContainer(GradientContainer & container) const;

  /// Gradient values for all variables using the same gradient method.
  struct LinearFVGradientContainer
  {
    /// Variable numbers whose gradients are stored in values.
    std::unordered_set<unsigned int> variable_numbers;

    /// Persistent gradient values read by consumers.
    GradientContainer values;
  };

  /**
   * Find or create the container associated with a gradient method.
   * @param method Gradient method whose container should be used.
   */
  LinearFVGradientContainer & linearFVGradientContainer(const FVGradientMethod & method);

  /**
   * Allocate persistent vectors for a registered gradient method.
   * @param container Method container whose values should be rebuilt.
   */
  void initializeLinearFVGradientValues(LinearFVGradientContainer & container);

  /**
   * Recompute the field values for a registered gradient method.
   * @param method Gradient method used to update the container.
   * @param container Method container whose values should be updated.
   */
  void updateLinearFVGradientContainer(const FVGradientMethod & method,
                                       LinearFVGradientContainer & container);

  /// Reference to the system object
  SystemBase & _sys;

  /// Reusable scratch space where a method writes replacement values before they are published.
  GradientContainer _linear_fv_gradient_output_scratch;

  /// Reusable scratch space available to the method while computing replacement values.
  GradientContainer _linear_fv_gradient_method_scratch;

  /// Gradient containers keyed by the method object that produces them.
  std::unordered_map<const FVGradientMethod *, LinearFVGradientContainer>
      _linear_fv_gradient_container_by_method;
};
