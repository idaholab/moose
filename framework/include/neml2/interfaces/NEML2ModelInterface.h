//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <thread>
#include <utility>
#include <tuple>
#include "NEML2Utils.h"
#include "InputParameters.h"

#ifdef NEML2_ENABLED
#include <ATen/Parallel.h>
#include "neml2/models/Model.h"
#include "neml2/base/Parser.h"
#include "neml2/dispatchers/WorkScheduler.h"
#include "neml2/dispatchers/WorkDispatcher.h"
#include "neml2/dispatchers/valuemap_helpers.h"
#include "neml2/dispatchers/derivmap_helpers.h"
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
  neml2::Model & model() const { return *_model; }

  /// Get the target compute device
  const torch::Device & device() const { return _device; }

  using RJType = std::tuple<neml2::ValueMap, neml2::DerivMap>;
  using DispatcherType =
      neml2::WorkDispatcher<neml2::ValueMap, RJType, RJType, neml2::ValueMap, RJType>;

  /// Get the work scheduler
  neml2::WorkScheduler * scheduler() { return _scheduler.get(); }
  /// Get the work dispatcher
  const std::unique_ptr<DispatcherType> & dispatcher() const { return _dispatcher; }

private:
  /// The device on which to evaluate the NEML2 model
  const torch::Device _device;
  /// The NEML2 factory
  std::unique_ptr<neml2::Factory> _factory;
  /// The NEML2 material model
  std::shared_ptr<neml2::Model> _model;

  /// The work scheduler to use
  std::shared_ptr<neml2::WorkScheduler> _scheduler;
  /// Work dispatcher
  std::unique_ptr<DispatcherType> _dispatcher;
  /// Whether to dispatch work asynchronously
  const bool _async_dispatch;
  /// Models for each thread
  std::unordered_map<std::thread::id, std::shared_ptr<neml2::Model>> _model_pool;

#endif // NEML2_ENABLED
};

template <class T>
InputParameters
NEML2ModelInterface<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addParam<DataFileName>(
      "input",
      NEML2Utils::docstring("Path to the NEML2 input file containing the NEML2 model(s)."));
  params.addParam<std::vector<std::string>>(
      "cli_args",
      {},
      NEML2Utils::docstring(
          "Additional command line arguments to use when parsing the NEML2 input file."));
  params.addParam<std::string>(
      "model",
      "",
      NEML2Utils::docstring("Name of the NEML2 model, i.e., the string inside the brackets [] in "
                            "the NEML2 input file that corresponds to the model you want to use."));
  params.addParam<std::string>(
      "device",
      "cpu",
      NEML2Utils::docstring(
          "Device on which to evaluate the NEML2 model. The string supplied must follow the "
          "following schema: (cpu|cuda)[:<device-index>] where cpu or cuda specifies the device "
          "type, and :<device-index> optionally specifies a device index. For example, "
          "device='cpu' sets the target compute device to be CPU, and device='cuda:1' sets the "
          "target compute device to be CUDA with device ID 1."));

  params.addParam<std::string>(
      "scheduler",
      NEML2Utils::docstring(
          "NEML2 scheduler to use to run the model.  If not specified no scheduler is used and "
          "MOOSE will pass all the constitutive updates to the provided device at once."));

  params.addParam<bool>(
      "async_dispatch", true, NEML2Utils::docstring("Whether to use asynchronous dispatch."));

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
    _device(params.get<std::string>("device")),
    _scheduler(nullptr),
    _async_dispatch(params.get<bool>("async_dispatch"))
{
  // Load model
  const auto & fname = params.get<DataFileName>("input");
  const auto & cli_args = params.get<std::vector<std::string>>("cli_args");
  _factory = neml2::load_input(std::string(fname), neml2::utils::join(cli_args, " "));
  _model = NEML2Utils::getModel(*_factory, params.get<std::string>("model"));
  _model->to(_device);

  // Load scheduler if specified
  if (params.isParamValid("scheduler"))
    _scheduler = _factory->get_scheduler(params.get<std::string>("scheduler"));

  if (_scheduler)
  {
    auto red = [](std::vector<RJType> && results) -> RJType
    {
      // Split into two separate vectors
      std::vector<neml2::ValueMap> vms;
      std::vector<neml2::DerivMap> dms;
      for (auto && [vm, dm] : results)
      {
        vms.push_back(std::move(vm));
        dms.push_back(std::move(dm));
      }
      return std::make_tuple(neml2::valuemap_cat_reduce(std::move(vms), 0),
                             neml2::derivmap_cat_reduce(std::move(dms), 0));
    };

    auto post = [this](RJType && x) -> RJType
    {
      return std::make_tuple(neml2::valuemap_move_device(std::move(std::get<0>(x)), _device),
                             neml2::derivmap_move_device(std::move(std::get<1>(x)), _device));
    };

    auto thread_init = [this](neml2::Device device) -> void
    {
      at::set_num_threads(libMesh::n_threads());
      at::set_num_interop_threads(libMesh::n_threads());
      auto model = NEML2Utils::getModel(*_factory, _model->name());
      model->to(device);
      _model_pool[std::this_thread::get_id()] = std::move(model);
    };

    _dispatcher = std::make_unique<DispatcherType>(
        *_scheduler,
        _async_dispatch,
        [&](neml2::ValueMap && x, neml2::Device device) -> RJType
        {
          auto & model =
              _async_dispatch ? libmesh_map_find(_model_pool, std::this_thread::get_id()) : _model;

          // If this is not an async dispatch, we need to move the model to the target device
          // _every_ time before evaluation
          if (!_async_dispatch)
            model->to(device);

          return model->value_and_dvalue(std::move(x));
        },
        red,
        &neml2::valuemap_move_device,
        post,
        _async_dispatch ? thread_init : std::function<void(neml2::Device)>());
  }
}

template <class T>
void
NEML2ModelInterface<T>::validateModel() const
{
  mooseAssert(_model != nullptr, "_model must be initialized");
  neml2::diagnose(*_model);
}

#endif // NEML2_ENABLED
