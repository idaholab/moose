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
   * Resolve a named gradient method, constructing a built-in method when needed.
   * @param method_name Name of the gradient method to retrieve.
   * @return Gradient method associated with the supplied name.
   */
  const FVGradientMethod & resolveFVGradientMethod(const GradientMethodName & method_name);

  /**
   * Register a variable for gradient values produced by a method object.
   * @param variable_number Variable number whose gradient should be stored.
   * @param method Gradient method that computes the field values.
   * @param oldest_state Oldest time state that consumers need to read.
   */
  LinearFVGradientReader registerFVGradient(unsigned int variable_number,
                                            const FVGradientMethod & method,
                                            unsigned int oldest_state = 0);

protected:
  /// One vector per spatial component of a cell-centered gradient field.
  using GradientContainer = LinearFVGradientReader::GradientContainer;

  /// Gradient fields indexed by solution time state.
  using GradientStateContainer = LinearFVGradientReader::GradientStateContainer;

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

  /**
   * Copy published gradient values into requested older time states.
   * @param skip_current_to_old Whether state zero should not overwrite state one.
   */
  void copyPreviousGradientStates(bool skip_current_to_old);

  /// Restore current gradients from state one after a failed timestep.
  void restoreGradientStates();

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

    /// Published gradient values indexed by solution time state.
    GradientStateContainer state_values;

    /// Replacement gradient values computed before publication.
    GradientContainer next_values;

    /// Whether the current gradient has received a computed value.
    bool current_state_initialized = false;
  };

  /**
   * Ensure that a method container stores every requested time state.
   * @param container Method container whose state storage should be grown.
   * @param oldest_state Oldest time state that must be stored.
   */
  void ensureGradientStateStorage(LinearFVGradientContainer & container, unsigned int oldest_state);

  /**
   * Copy one complete gradient field.
   * @param source Gradient field to copy.
   * @param destination Gradient field to overwrite.
   */
  void copyGradient(const GradientContainer & source, GradientContainer & destination) const;

  /// Compute and publish uninitialized gradients before their first time-state advancement.
  void initializeGradientStatesForTimeAdvance();

  /**
   * Compute replacement field values for a registered gradient method.
   * @param method Gradient method used to compute the replacement values.
   * @return Method container whose replacement values were computed.
   */
  LinearFVGradientContainer & computeLinearFVGradientContainer(const FVGradientMethod & method);

  /**
   * Replace the current gradient storage with the freshly computed new gradients.
   * @param container Method container whose gradient values should be finalized.
   */
  void finalizeLinearFVGradientContainer(LinearFVGradientContainer & container);

  /// Reference to the system object
  SystemBase & _sys;

  /// Gradient containers keyed by the method object that produces them.
  std::unordered_map<const FVGradientMethod *, LinearFVGradientContainer>
      _linear_fv_gradient_container_by_method;
};
