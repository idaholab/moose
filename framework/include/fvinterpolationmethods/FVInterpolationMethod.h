//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "FaceInfo.h"
#include "GradientLimiterType.h"
#include "MooseFunctorForward.h"
#include "MooseError.h"

#include <cstddef>
#include <cstring>
#include <type_traits>

template <typename>
class MooseLinearVariableFV;

/**
 * Base class for face interpolation functions used by linear FV objects.
 * Maybe we can extend it a little later to regular FV as well.
 *
 * These objects expose lightweight callable interpolators that can be cached by kernels so
 * that interpolation does not require virtual dispatch inside hot loops. Derived classes only need
 * to implement the actual interpolation formula and register it through the helper methods provided
 * here.
 *
 * The logic is a little twisted here to enable SIMD and later Kokkos vectorization (efficient
 * loops). We need this to be as lightweight as possible and we have to make sure it is trivially
 * copyable.
 *
 * Another reason why this is a little complicated is that we would like to make sure
 * the derived classes can use their own member variables (like a power variable for inverse
 * distance weighting, or a limiter variable etc).
 */
class FVInterpolationMethod : public MooseObject
{
public:
  static InputParameters validParams();

  FVInterpolationMethod(const InputParameters & params);

  /// Storage reserved for trivially-copyable handle payloads.
  static constexpr std::size_t handle_storage_bytes = 64;

  /**
   * Lightweight callable container used to evaluate the interpolation at a face.
   * This is used for generic interpolation (e.g. diffusion coefficients).
   */
  struct FaceInterpolator
  {
    /// Function pointer used for the face evaluation
    using Eval = Real (*)(const void *, const FaceInfo &, Real, Real);

    /// Used for checking if the interpolator exists before we evaluate it
    bool valid() const { return eval; }

    template <typename Data>
    void set(const Data & data, Eval eval_in)
    {
      static_assert(std::is_trivially_copyable_v<Data>,
                    "Interpolation handle data must be trivially copyable");
      static_assert(std::is_trivially_destructible_v<Data>,
                    "Interpolation handle data must be trivially destructible");
      static_assert(sizeof(Data) <= handle_storage_bytes,
                    "Interpolation handle data too large for storage");
      std::memset(_storage, 0, sizeof(_storage));
      if constexpr (!std::is_empty_v<Data>)
        std::memcpy(_storage, &data, sizeof(Data));
      eval = eval_in;
    }

    /// Operator for convenient evaluation, just calls eval with forwarded arguments
    Real operator()(const FaceInfo & face, Real elem_value, Real neighbor_value) const
    {
      mooseAssert(valid(), "Attempting to call an empty interpolation handle");
      return eval(_storage, face, elem_value, neighbor_value);
    }

    /// Convenience interface: interpolate a functor to a face.
    Real operator()(const Moose::FunctorBase<Real> & functor,
                    const FaceInfo & face,
                    const Moose::StateArg & state) const;

    /// Convenience interface: interpolate a linear FV variable to a face.
    Real operator()(const MooseLinearVariableFV<Real> & var,
                    const FaceInfo & face,
                    const Moose::StateArg & state) const;

  private:
    /// Sorry for the alignas: without max alignment this byte buffer would be 1-byte aligned,
    /// and placing data with stricter alignment (e.g., double or VectorValue) would be
    /// undefined behavior. This storage lets us pass interpolation parameters without a host
    /// pointer while staying safely aligned for any trivially-copyable data.
    alignas(std::max_align_t) unsigned char _storage[handle_storage_bytes] = {};

    /// Function pointer that performs the interpolation
    Eval eval = nullptr;
  };

  /// Callable container used to evaluate a single advected face value.
  struct AdvectedValueInterpolator
  {
    using Eval = Real (*)(const void *,
                          const FaceInfo &,
                          Real,
                          Real,
                          const VectorValue<Real> *,
                          const VectorValue<Real> *,
                          Real);

    bool valid() const { return eval; }
    bool needsGradients() const { return _needs_gradients; }
    bool needsLimitedGradients() const
    {
      return _gradient_limiter != Moose::FV::GradientLimiterType::None;
    }
    Moose::FV::GradientLimiterType gradientLimiter() const { return _gradient_limiter; }

    void setNeedsGradients(const bool needs_gradients) { _needs_gradients = needs_gradients; }
    void setGradientLimiter(const Moose::FV::GradientLimiterType limiter)
    {
      _gradient_limiter = limiter;
    }

    template <typename Data>
    void set(const Data & data, Eval eval_in)
    {
      static_assert(std::is_trivially_copyable_v<Data>,
                    "Interpolation handle data must be trivially copyable");
      static_assert(std::is_trivially_destructible_v<Data>,
                    "Interpolation handle data must be trivially destructible");
      static_assert(sizeof(Data) <= handle_storage_bytes,
                    "Interpolation handle data too large for storage");
      std::memset(_storage, 0, sizeof(_storage));
      if constexpr (!std::is_empty_v<Data>)
        std::memcpy(_storage, &data, sizeof(Data));
      eval = eval_in;
    }

    Real operator()(const FaceInfo & face,
                    Real elem_value,
                    Real neighbor_value,
                    const VectorValue<Real> * elem_grad,
                    const VectorValue<Real> * neighbor_grad,
                    Real mass_flux) const
    {
      mooseAssert(valid(), "Attempting to call an empty advected value interpolation handle");
      mooseAssert(!needsGradients() || elem_grad,
                  "Gradient required by advected value interpolation but elem_grad is null");
      return eval(_storage, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
    }

    /// Convenience interface: interpolate a functor to a face with an upwind decision based on
    /// the supplied mass flux.
    Real operator()(const Moose::FunctorBase<Real> & functor,
                    const FaceInfo & face,
                    const Moose::StateArg & state,
                    Real mass_flux) const;

    /// Convenience interface: interpolate a linear FV variable to a face with an upwind decision
    /// based on the supplied mass flux.
    Real operator()(const MooseLinearVariableFV<Real> & var,
                    const FaceInfo & face,
                    const Moose::StateArg & state,
                    Real mass_flux) const;

  private:
    /// Sorry for the alignas: without max alignment this byte buffer would be 1-byte aligned,
    /// and placing data with stricter alignment (e.g., double or VectorValue) would be
    /// undefined behavior. This storage lets us pass interpolation parameters without a host
    /// pointer while staying safely aligned for any trivially-copyable data.
    alignas(std::max_align_t) unsigned char _storage[handle_storage_bytes] = {};
    Eval eval = nullptr;
    bool _needs_gradients = false;
    Moose::FV::GradientLimiterType _gradient_limiter = Moose::FV::GradientLimiterType::None;
  };

  /// Matrix/RHS contribution for an advected face interpolation.
  struct AdvectedSystemContribution
  {
    std::pair<Real, Real> weights_matrix;
    Real rhs_face_value = 0.0;
  };

  /// Callable container used to evaluate advection-specific matrix/RHS contributions.
  struct AdvectedSystemContributionCalculator
  {
    using Eval = AdvectedSystemContribution (*)(const void *,
                                                const FaceInfo &,
                                                Real,
                                                Real,
                                                const VectorValue<Real> *,
                                                const VectorValue<Real> *,
                                                Real);

    bool valid() const { return eval; }
    bool needsGradients() const { return _needs_gradients; }
    bool needsLimitedGradients() const
    {
      return _gradient_limiter != Moose::FV::GradientLimiterType::None;
    }
    Moose::FV::GradientLimiterType gradientLimiter() const { return _gradient_limiter; }

    void setNeedsGradients(const bool needs_gradients) { _needs_gradients = needs_gradients; }
    void setGradientLimiter(const Moose::FV::GradientLimiterType limiter)
    {
      _gradient_limiter = limiter;
    }

    template <typename Data>
    void set(const Data & data, Eval eval_in)
    {
      static_assert(std::is_trivially_copyable_v<Data>,
                    "Interpolation handle data must be trivially copyable");
      static_assert(std::is_trivially_destructible_v<Data>,
                    "Interpolation handle data must be trivially destructible");
      static_assert(sizeof(Data) <= handle_storage_bytes,
                    "Interpolation handle data too large for storage");
      std::memset(_storage, 0, sizeof(_storage));
      if constexpr (!std::is_empty_v<Data>)
        std::memcpy(_storage, &data, sizeof(Data));
      eval = eval_in;
    }

    AdvectedSystemContribution operator()(const FaceInfo & face,
                                          Real elem_value,
                                          Real neighbor_value,
                                          const VectorValue<Real> * elem_grad,
                                          const VectorValue<Real> * neighbor_grad,
                                          Real mass_flux) const
    {
      mooseAssert(valid(), "Attempting to call an empty advected interpolation handle");
      mooseAssert(!needsGradients() || elem_grad,
                  "Gradient required by advected interpolation but elem_grad is null");
      return eval(_storage, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
    }

    /// Convenience interface: compute advected contributions using a functor sampled on each side
    /// of the face.
    AdvectedSystemContribution operator()(const Moose::FunctorBase<Real> & functor,
                                          const FaceInfo & face,
                                          const Moose::StateArg & state,
                                          Real mass_flux) const;

    /// Convenience interface: compute advected contributions using a linear FV variable sampled
    /// on each side of the face.
    AdvectedSystemContribution operator()(const MooseLinearVariableFV<Real> & var,
                                          const FaceInfo & face,
                                          const Moose::StateArg & state,
                                          Real mass_flux) const;

  private:
    /// Sorry for the alignas: without max alignment this byte buffer would be 1-byte aligned,
    /// and placing data with stricter alignment (e.g., double or VectorValue) would be
    /// undefined behavior. This storage lets us pass interpolation parameters without a host
    /// pointer while staying safely aligned for any trivially-copyable data.
    alignas(std::max_align_t) unsigned char _storage[handle_storage_bytes] = {};
    Eval eval = nullptr;
    bool _needs_gradients = false;
    Moose::FV::GradientLimiterType _gradient_limiter = Moose::FV::GradientLimiterType::None;
  };

  /**
   * @return The face interpolation callable associated with this user object.
   */
  FaceInterpolator faceInterpolator() const { return _face_interpolator; }

  AdvectedValueInterpolator advectedFaceValueInterpolator() const
  {
    return _advected_face_value_interpolator;
  }

  AdvectedSystemContributionCalculator advectedSystemContributionCalculator() const
  {
    return _advected_system_contribution_calculator;
  }

protected:
  /**
   * Utility to build an interpolation function for Derived classes.
   */
  template <typename Derived, typename Data>
  FaceInterpolator buildFaceInterpolator(const Data & data) const
  {
    FaceInterpolator interpolator;
    interpolator.set(data, &FVInterpolationMethod::callInterpolate<Derived, Data>);
    return interpolator;
  }

  /**
   * Utility to build an advected face value interpolation function for Derived classes.
   */
  template <typename Derived, typename Data>
  AdvectedValueInterpolator buildAdvectedFaceValueInterpolator(const Data & data,
                                                               const bool needs_gradients) const
  {
    AdvectedValueInterpolator interpolator;
    interpolator.set(data, &FVInterpolationMethod::callAdvectedInterpolateValue<Derived, Data>);
    interpolator.setNeedsGradients(needs_gradients);
    return interpolator;
  }

  /**
   * Utility to build an advected face value interpolation function for Derived classes where the
   * limiter type is encapsulated by the Derived class (e.g. a member variable populated from input
   * parameters).
   *
   * The Derived class must provide:
   *   Moose::FV::GradientLimiterType gradientLimiter() const;
   */
  template <typename Derived, typename Data>
  AdvectedValueInterpolator buildAdvectedFaceValueInterpolatorLimited(const Data & data) const
  {
    auto interpolator = buildAdvectedFaceValueInterpolator<Derived>(data, true);
    interpolator.setGradientLimiter(static_cast<const Derived &>(*this).gradientLimiter());
    return interpolator;
  }

  /**
   * Utility to build an advected interpolation function for Derived classes.
   */
  template <typename Derived, typename Data>
  AdvectedSystemContributionCalculator
  buildAdvectedSystemContributionCalculator(const Data & data, const bool needs_gradients) const
  {
    AdvectedSystemContributionCalculator interpolator;
    interpolator.set(data, &FVInterpolationMethod::callAdvectedSystemContribution<Derived, Data>);
    interpolator.setNeedsGradients(needs_gradients);
    return interpolator;
  }

  /**
   * Utility to build an advected interpolation function for Derived classes where the limiter type
   * is encapsulated by the Derived class.
   *
   * The Derived class must provide:
   *   Moose::FV::GradientLimiterType gradientLimiter() const;
   */
  template <typename Derived, typename Data>
  AdvectedSystemContributionCalculator
  buildAdvectedSystemContributionCalculatorLimited(const Data & data) const
  {
    auto interpolator = buildAdvectedSystemContributionCalculator<Derived>(data, true);
    interpolator.setGradientLimiter(static_cast<const Derived &>(*this).gradientLimiter());
    return interpolator;
  }

  /**
   * Save a fully constructed interpolator so kernels can evaluate it later.
   */
  void setFaceInterpolator(FaceInterpolator interpolator) { _face_interpolator = interpolator; }

  /**
   * Save a fully constructed advected face value interpolator.
   */
  void setAdvectedFaceValueInterpolator(AdvectedValueInterpolator interpolator)
  {
    _advected_face_value_interpolator = interpolator;
  }

  /**
   * Save a fully constructed advected interpolator so advection kernels can evaluate it later.
   */
  void setAdvectedSystemContributionCalculator(AdvectedSystemContributionCalculator interpolator)
  {
    _advected_system_contribution_calculator = interpolator;
  }

private:
  /// Wrapper for face interpolation calls (kept adjacent to other call wrappers)
  template <typename Derived, typename Data>
  static Real callInterpolate(const void * data,
                              const FaceInfo & face,
                              const Real elem_value,
                              const Real neighbor_value)
  {
    return Derived::interpolate(*static_cast<const Data *>(data), face, elem_value, neighbor_value);
  }

  /**
   * Wrapper for advected value interpolation calls.
   */
  template <typename Derived, typename Data>
  static Real callAdvectedInterpolateValue(const void * data,
                                           const FaceInfo & face,
                                           const Real elem_value,
                                           const Real neighbor_value,
                                           const VectorValue<Real> * elem_grad,
                                           const VectorValue<Real> * neighbor_grad,
                                           const Real mass_flux)
  {
    return Derived::advectedInterpolateValue(*static_cast<const Data *>(data),
                                             face,
                                             elem_value,
                                             neighbor_value,
                                             elem_grad,
                                             neighbor_grad,
                                             mass_flux);
  }

  /**
   * Wrapper for advected interpolation calls to allow member access while staying trivially
   * copyable.
   */
  template <typename Derived, typename Data>
  static AdvectedSystemContribution
  callAdvectedSystemContribution(const void * data,
                                 const FaceInfo & face,
                                 const Real elem_value,
                                 const Real neighbor_value,
                                 const VectorValue<Real> * elem_grad,
                                 const VectorValue<Real> * neighbor_grad,
                                 const Real mass_flux)
  {
    return Derived::advectedInterpolate(*static_cast<const Data *>(data),
                                        face,
                                        elem_value,
                                        neighbor_value,
                                        elem_grad,
                                        neighbor_grad,
                                        mass_flux);
  }

  FaceInterpolator _face_interpolator;
  AdvectedValueInterpolator _advected_face_value_interpolator;
  AdvectedSystemContributionCalculator _advected_system_contribution_calculator;
};
