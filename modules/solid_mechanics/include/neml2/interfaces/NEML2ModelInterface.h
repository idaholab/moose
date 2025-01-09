//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#include "neml2/misc/parser_utils.h"
#include "Material.h"
#include "UserObject.h"
#endif

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
   * Validate the NEML2 material model. Note that the developer is responsible for calling this
   * method at the appropriate times, for example, at initialSetup().
   */
  virtual void validateModel() const;

  /// Get the NEML2 model
  neml2::Model & model() const { return _model; }

  /// Get the target compute device
  const torch::Device & device() const { return _device; }

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
  params.addParam<std::string>(
      "model",
      "",
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
  return params;
}

#ifndef NEML2_ENABLED

template <class T>
template <typename... P>
NEML2ModelInterface<T>::NEML2ModelInterface(const InputParameters & params, P &&... args)
  : T(params, args...)
{
}

#else

template <class T>
template <typename... P>
NEML2ModelInterface<T>::NEML2ModelInterface(const InputParameters & params, P &&... args)
  : T(params, args...),
    _model(neml2::get_model(params.get<std::string>("model"))),
    _device(params.get<std::string>("device"))
{
  _model.to(_device);
}

template <class T>
void
NEML2ModelInterface<T>::validateModel() const
{
  neml2::diagnose(_model);
}

#endif // NEML2_ENABLED
