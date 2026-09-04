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

#include <memory>
#include <string>
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

  /// Gradient fields indexed by solution time state.
  using GradientStateContainer = std::vector<GradientContainer>;

  /**
   * @param sys System that owns the variables and gradient values.
   * @param state_values Gradient fields indexed by solution time state.
   * @param method Gradient method that produces the values read by this object.
   * @param variable_number Variable number whose gradient this object reads.
   */
  LinearFVGradientReader(const SystemBase & sys,
                         const GradientStateContainer & state_values,
                         const FVGradientMethod & method,
                         unsigned int variable_number);

  /// Access the underlying component vectors keyed by spatial direction.
  const GradientContainer & components() const;

  /**
   * Access the component vectors for a solution state.
   * @param state State whose gradient components should be accessed.
   */
  const GradientContainer & components(const Moose::StateArg & state) const;

  /// System whose DOF map indexes the stored values.
  const SystemBase & system() const { return _sys; }

  /// Method object that produces the stored values.
  const FVGradientMethod & method() const { return _method; }

  /**
   * Read one gradient component at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param index Index of the spatial component of the gradient.
   */
  Real component(const ElemInfo & elem_info, unsigned int index) const;

  /**
   * Read one gradient component at an element and solution state.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param index Index of the spatial component of the gradient.
   * @param state State whose gradient should be read.
   */
  Real
  component(const ElemInfo & elem_info, unsigned int index, const Moose::StateArg & state) const;

  /**
   * Read the full gradient at an element.
   * @param elem_info Element whose cell-centered gradient should be read.
   */
  RealVectorValue gradient(const ElemInfo & elem_info) const;

  /**
   * Read the full gradient at an element and solution state.
   * @param elem_info Element whose cell-centered gradient should be read.
   * @param state State whose gradient should be read.
   */
  RealVectorValue gradient(const ElemInfo & elem_info, const Moose::StateArg & state) const;

  /**
   * Read the full gradient interpolated to a face.
   * @param fi Face whose interpolated gradient should be read.
   */
  RealVectorValue gradient(const FaceInfo & fi) const;

  /**
   * Read the full gradient interpolated to a face at a solution state.
   * @param fi Face whose interpolated gradient should be read.
   * @param state State whose gradient should be read.
   */
  RealVectorValue gradient(const FaceInfo & fi, const Moose::StateArg & state) const;

private:
  /// Validate and select the component vectors for a solution state.
  const GradientContainer & stateComponents(const Moose::StateArg & state) const;

  /// System the variable belongs to.
  const SystemBase & _sys;

  /// System number cached for hot lookups (from faces to dofs).
  const unsigned int _system_number;

  /// Gradient fields indexed by solution time state.
  const GradientStateContainer & _state_values;

  /// Method object that produces the stored values.
  const FVGradientMethod & _method;

  /// Variable number whose gradients are read by this object.
  const unsigned int _variable_number;
};
