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
#include "LinearFVGradientReader.h"

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
 * Registration, update, and allocation logic for linear finite-volume cell gradients. This
 * interface should be inherited by system classes that may own linear finite-volume variables
 */
class LinearFVGradientInterface
{
public:
  /**
   * @param sys System that owns registered linear finite-volume gradient fields.
   */
  LinearFVGradientInterface(SystemBase & sys) : _sys(sys) {}

  /**
   * Register a variable for gradient values produced by a method object.
   * @param variable_number Variable number whose gradient should be stored.
   * @param method Gradient method that computes the field values.
   */
  LinearFVGradientReader registerFVGradient(unsigned int variable_number,
                                            const FVGradientMethod & method);

protected:
  /// One vector per spatial component of a cell-centered gradient field.
  using GradientContainer = LinearFVGradientReader::GradientContainer;

  /// Compute and finalize all registered linear FV gradient fields.
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

  /// Whether any linear finite-volume gradient fields have been registered to this object.
  bool hasLinearFVGradients() const;

  /**
   * Allocate one zeroed vector per spatial component for gradient storage.
   * @param container Component-vector container to rebuild.
   */
  void initializeContainer(GradientContainer & container) const;

  /// Gradient values for all variables using the same gradient method.
  struct LinearFVGradientContainer
  {
    /// Variable numbers whose gradients are stored in the gradient containers.
    std::unordered_set<unsigned int> variable_numbers;

    /// Current gradient values read by consumers.
    GradientContainer values;

    /// Replacement gradient values computed before publication.
    GradientContainer next_values;
  };

  /**
   * Compute replacement field values for a registered gradient method.
   * @param method Gradient method used to compute the replacement values.
   * @param container Method container whose next values should be computed.
   */
  void computeLinearFVGradientContainer(const FVGradientMethod & method,
                                        LinearFVGradientContainer & container);

  /**
   * Replace the current gradient storage with the freshly computed new gradients.
   * @param container Method container whose gradient values should be finalized.
   */
  void finalizeLinearFVGradientContainer(LinearFVGradientContainer & container);

  /// Reference to the system object
  SystemBase & _sys;

  /// Reusable scratch space available to the method while computing replacement values.
  GradientContainer _linear_fv_gradient_method_scratch;

  /// Gradient containers keyed by the method object that produces them.
  std::unordered_map<const FVGradientMethod *, LinearFVGradientContainer>
      _linear_fv_gradient_container_by_method;
};
