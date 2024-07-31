//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#include "neml2/misc/parser_utils.h"
#include "Material.h"
#include "UserObject.h"
#endif

#include "NEML2Utils.h"

/**
 * Interface class to provide common input parameters, members, and methods for MOOSEObjects that
 * use NEML2 models.
 */
template <class T>
class NEML2ModelInterface : public T
{
public:
  static InputParameters validParams();

  template <typename... P>
  NEML2ModelInterface(const InputParameters & params, P &&... args);

#ifdef NEML2_ENABLED

protected:
  /**
   * Validate the NEML2 material model. This method should throw a moose error with the first
   * encountered problem. Note that the developer is responsible for calling this method at the
   * appropriate times, for example, at initialSetup().
   */
  virtual void validateModel() const;

  /// Initialize the model with a batch shape
  void initModel(neml2::TensorShapeRef batch_shape);

  /// Get the NEML2 model
  neml2::Model & model() const { return _model; }

  /// Get the target compute device
  const torch::Device & device() const { return _device; }

  /**
   * @brief Convert a raw string to a neml2::VariableName
   *
   * @param raw_str
   * @return neml2::VariableName
   */
  neml2::VariableName getNEML2VariableName(const std::string & raw_str) const;

private:
  /// The NEML2 material model
  neml2::Model & _model;

  /// The device on which to evaluate the NEML2 model
  const torch::Device _device;

#endif // NEML2_ENABLED
};

template <class T>
InputParameters
NEML2ModelInterface<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addRequiredParam<std::string>(
      "model",
      "Name of the NEML2 model, i.e., the string inside the brackets [] in the NEML2 input file "
      "that corresponds to the model you want to use.");
  params.addParam<std::string>(
      "device",
      "cpu",
      "Device on which to evaluate the NEML2 model. The string supplied must follow the following "
      "schema: (cpu|cuda)[:<device-index>] where cpu or cuda specifies the device type, and "
      ":<device-index> optionally specifies a device index. For example, device='cpu' sets the "
      "target compute device to be CPU, and device='cuda:1' sets the target compute device to be "
      "CUDA with device ID 1.");
  params.addParam<bool>("enable_AD",
                        false,
                        "Set to true to enable PyTorch AD. When set to false (default), no "
                        "function graph or gradient is computed, which speeds up model "
                        "evaluation.");
  return params;
}

#ifndef NEML2_ENABLED

template <class T>
template <typename... P>
NEML2ModelInterface<T>::NEML2ModelInterface(const InputParameters & params, P &&... args)
  : T(params, args...)
{
  NEML2Utils::libraryNotEnabledError(params);
}

#else

template <class T>
template <typename... P>
NEML2ModelInterface<T>::NEML2ModelInterface(const InputParameters & params, P &&... args)
  : T(params, args...),
    _model(neml2::get_model(params.get<std::string>("model"), params.get<bool>("enable_AD"))),
    _device(params.get<std::string>("device"))
{
}

template <class T>
void
NEML2ModelInterface<T>::validateModel() const
{
  neml2::diagnose(_model);
}

template <class T>
void
NEML2ModelInterface<T>::initModel(neml2::TensorShapeRef batch_shape)
{
  _model.reinit(batch_shape, /*deriv_order=*/1, _device, /*dtype=*/torch::kFloat64);
}

template <class T>
neml2::VariableName
NEML2ModelInterface<T>::getNEML2VariableName(const std::string & raw_str) const
{
  return neml2::utils::parse<neml2::VariableName>(raw_str);
}

#endif // NEML2_ENABLED
