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
#include "NEML2ModelHandle.h"
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

  /// Get the NEML2 model handle (cpp-aoti by default, cpp-eager when eager=true)
  const NEML2ModelHandle & model() const { return *_model; }

  /// Non-const access to the NEML2 model handle, for runtime parameter writes (set_parameter)
  NEML2ModelHandle & model() { return *_model; }

  /// Get the target compute device
  const at::Device & device() const { return _device; }

  /// Get the target output device
  const at::Device & output_device() const { return _output_device; }

private:
  /// Resolve the 'load' DataFileName list (whose relative paths InputParameters has already
  /// resolved against the input file + data search path) to plain path strings for the eager
  /// runtime's external-extension loader.
  static std::vector<std::string> loadExtensionPaths(const InputParameters & params)
  {
    const auto files = params.get<std::vector<DataFileName>>("load");
    return std::vector<std::string>(files.begin(), files.end());
  }

  /// Parse the runtime parameters and build the model handle. Static so it can run in the member
  /// initializer list (before _device, which is read back from the handle). `self` supplies the
  /// MooseObject services (paramError, comm, app) once the T base is constructed.
  static std::unique_ptr<NEML2ModelHandle> makeModelHandle(const InputParameters & params,
                                                           NEML2ModelInterface<T> & self)
  {
    const bool eager = params.get<bool>("eager");
    const auto load = loadExtensionPaths(params);

    // 'load' imports Python extensions into the embedded interpreter -- cpp-eager only.
    if (!eager && !load.empty())
      self.paramError("load",
                      "The 'load' parameter imports Python extensions into the embedded "
                      "interpreter and is only valid with the cpp-eager runtime. Set eager=true to "
                      "use it, or remove it for the cpp-aoti runtime.");
    // 'input' is the source .i (cpp-eager) or the compiled-artifact stub .i (cpp-aoti).
    if (!params.isParamValid("input"))
      self.paramError("input", "'input' (the NEML2 source or AOTI stub '.i') is required.");

    // Device list (default: the app's libtorch compute device).
    auto devices = params.get<std::vector<std::string>>("device");
    if (devices.empty())
      devices = {at::Device(self.getMooseApp().getLibtorchDevice()).str()};

    // Per-device chunk sizes (default 0 = whole batch); length 1 broadcasts, else matches 'device'.
    auto db = params.get<std::vector<unsigned int>>("device_batch");
    if (db.empty())
      db = {0};
    if (db.size() != 1 && db.size() != devices.size())
      self.paramError("device_batch",
                      "'device_batch' must have length 1 or the same length as 'device'.");
    const std::vector<std::size_t> batch_sizes(db.begin(), db.end());

    // Host MPI communicator for the cpp-aoti CUDA scheduler (read at construction only; unused for
    // cpp-eager and for CPU dispatch).
    const auto mpi_comm = self.comm().get();

    return makeNEML2ModelHandle(eager,
                                std::string(params.get<DataFileName>("input")),
                                params.get<std::string>("model"),
                                devices,
                                batch_sizes,
                                &mpi_comm,
                                load);
  }

  /// The NEML2 material model handle. Defaults to the cpp-aoti runtime (loads an ahead-of-time-
  /// compiled artifact from the 'input' stub); the cpp-eager runtime (embeds a CPython interpreter
  /// and loads the model from the source 'input' file) is opt-in via the 'eager' parameter.
  std::unique_ptr<NEML2ModelHandle> _model;
  /// The device on which to evaluate the NEML2 model (this rank's scheduler-assigned device)
  const at::Device _device;
  /// The device on which to store the outputs
  const at::Device _output_device;

#endif // NEML2_ENABLED
};

template <class T>
InputParameters
NEML2ModelInterface<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addParam<bool>(
      "eager",
      false,
      "Use the cpp-eager NEML2 runtime, which embeds a Python interpreter and loads the model "
      "from the source 'input' file at runtime. Default false -- use the cpp-aoti runtime, which "
      "loads an ahead-of-time-compiled artifact (no Python at runtime) from the compile stub "
      "given by 'input'. The 'load' parameter is only valid with eager=true.");
  params.addParam<DataFileName>(
      "input",
      "Path to the NEML2 '.i' file: the source model file for the cpp-eager runtime, or the "
      "'<model>_aoti.i' stub produced by neml2-compile for the cpp-aoti runtime.");
  params.addParam<std::vector<std::string>>(
      "cli_args",
      {},
      "Additional command line arguments to use when parsing the NEML2 input file.");
  params.addParam<std::vector<DataFileName>>(
      "load",
      {},
      "External Python extension files (paths to .py files or package directories, resolved "
      "relative to the input file and the application's data search path) imported into the "
      "embedded interpreter before the NEML2 model is built. Importing them registers any "
      "@register_neml2_object types they define -- e.g. NEML2 models hosted inside a MOOSE app -- "
      "so the NEML2 input file can reference them. Mirrors the neml2 CLI --load flag. Only valid "
      "with the cpp-eager runtime (eager=true).");
  params.addParam<std::string>(
      "model",
      "",
      "Name of the NEML2 model, i.e., the string inside the brackets [] in the NEML2 input file "
      "that corresponds to the model you want to use.");
  params.addParam<std::vector<std::string>>(
      "device",
      {},
      "Compute device(s) for the NEML2 model, each following the schema (cpu|cuda)[:<device-index>] "
      "(e.g. 'cpu', 'cuda:1'). cpp-eager pins the model to the first device. cpp-aoti dispatches "
      "over the list: a CUDA list is round-robined over the MPI ranks on each node (one device per "
      "rank when counts match, sharing when there are more ranks than devices, an error when there "
      "are fewer); a single 'cpu' runs each rank's local batch on CPU. If not specified, defaults "
      "to the compute device from the --compute-device command line argument.");
  params.addParam<std::vector<unsigned int>>(
      "device_batch",
      {},
      "Per-device chunk size along the leading batch axis (0 = run the whole local batch at once). "
      "Must have length 1 (broadcast to all devices) or match the length of 'device'. Defaults to "
      "0.");
  params.addParam<std::string>(
      "output_device",
      "Device on which to store the outputs, following the same schema as a single 'device' entry. "
      "Defaults to the model's compute device.");

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
    // The cpp-eager runtime loads the model from the source .i (embedding CPython); the cpp-aoti
    // runtime (default) loads the ahead-of-time-compiled artifact stub via neml2::aoti::load_model.
    // makeModelHandle parses the runtime parameters; _device is the handle's resolved device.
    _model(makeModelHandle(params, *this)),
    _device(_model->device()),
    _output_device(params.isParamValid("output_device")
                       ? at::Device(params.get<std::string>("output_device"))
                       : _device)
{
}

template <class T>
void
NEML2ModelInterface<T>::validateModel() const
{
  // NEML2 v3 provides no C++ diagnose() equivalent; constructing the eager model above already
  // parses and validates it. Kept as a hook for derived classes.
}

#endif // NEML2_ENABLED
