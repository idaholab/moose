//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_LIBTORCH_ENABLED

#include "FunctorMaterial.h"
#include "LinearInterpolation.h"
#include "TorchScriptUserObject.h"

#include <memory>
#include <vector>

/**
 * Evaluates a TorchScript model once during initialSetup(), interprets its output
 * as one or more sampled one-dimensional profiles, and publishes those profiles
 * as Real functors.
 *
 * Supported TorchScript output shapes:
 *
 *   [N]       one profile
 *   [C, N]    C profiles
 *   [1, C, N] batched C-profile output
 *
 * where N must equal profile_coordinates.size() and C must equal
 * profile_names.size().
 *
 * The spatial profile coordinate is
 *
 *   s = ((point - profile_origin) dot profile_direction) / coordinate_scale
 *
 * where profile_direction is normalized internally.
 */
class TorchScriptProfileFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  TorchScriptProfileFunctorMaterial(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  /**
   * Construct the model input tensor.
   */
  torch::Tensor buildInputTensor() const;

  /**
   * Evaluate a cached profile at a physical point.
   */
  Real sampleProfile(unsigned int profile_index, const Point & point) const;

  /// Existing MOOSE object that owns and evaluates the TorchScript module
  const TorchScriptUserObject & _torch_script_userobject;

  /// Postprocessor-based model inputs
  const std::vector<PostprocessorName> _input_names;

  /// Direct constant model inputs
  const std::vector<Real> _input_values;

  /// References to postprocessor values, populated when input_names is used
  std::vector<const PostprocessorValue *> _postprocessor_inputs;

  /// Names assigned to the output profile functors
  const std::vector<MooseFunctorName> _profile_names;

  /// Coordinates at which the model output values are defined
  const std::vector<Real> _profile_coordinates;

  /// Physical origin of the one-dimensional profile coordinate
  const Point _profile_origin;

  /// Unit direction of increasing profile coordinate
  RealVectorValue _profile_direction;

  /// Physical distance corresponding to one profile-coordinate unit
  const Real _coordinate_scale;

  /// error, clamp, or extrapolate
  const MooseEnum _out_of_range_behavior;

  /// float32 or float64
  const MooseEnum _tensor_dtype;

  /// Cached interpolator for each model output channel
  std::vector<std::unique_ptr<LinearInterpolation>> _profiles;
};

#endif
