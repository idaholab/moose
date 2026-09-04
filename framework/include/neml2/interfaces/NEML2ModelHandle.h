//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"

#ifdef NEML2_ENABLED

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <cstddef>
#include <filesystem>

#include "neml2/csrc/eager/Model.h"
#include "neml2/csrc/eager/load_model.h"
#include "neml2/csrc/dispatchers/factory.h"
#include "neml2/csrc/dispatchers/DispatchedModel.h"
#include "neml2/csrc/dispatchers/MPISimpleScheduler.h"

/**
 * Runtime-agnostic handle over a NEML2 v3 model.
 *
 * NEML2 v3 ships two C++ runtimes with parallel APIs but no common base class:
 * `neml2::eager::Model` (embeds CPython, loads a model from a source `.i`) and the cpp-aoti
 * `neml2::aoti::DispatchedModel` (loads ahead-of-time-compiled artifacts -- a shared
 * `metadata.json` plus per-`<device>/<dtype>/` `.pt2` binaries -- straight from the artifact
 * folder, Python-free, dispatched to a device by a scheduler). This handle exposes the single
 * surface the MOOSE integration needs -- introspection (names + base shapes) and evaluation
 * (jacobian / param_jacobian / set_parameter) -- so both the setup-time introspection in
 * NEML2Action and the runtime evaluation in NEML2ModelExecutor use one consistent API regardless of
 * which runtime backs the model.
 *
 * Construct one with ::makeNEML2ModelHandle.
 */
class NEML2ModelHandle
{
public:
  virtual ~NEML2ModelHandle() = default;

  /// @name Introspection, in graph-call order (params keyed by qualified name)
  ///@{
  virtual const std::vector<std::string> & input_names() const = 0;
  virtual const std::vector<std::string> & output_names() const = 0;
  virtual const std::vector<std::vector<int64_t>> & input_base_shapes() const = 0;
  virtual const std::vector<std::vector<int64_t>> & output_base_shapes() const = 0;
  virtual const std::map<std::string, std::vector<int64_t>> & parameter_base_shapes() const = 0;
  ///@}

  /// @name Evaluation
  ///@{
  /// Evaluate the model only (outputs, no derivatives). Used on pure residual evaluations -- line
  /// search, damping, JFNK residuals -- to skip the (comparatively expensive) derivative.
  virtual std::map<std::string, at::Tensor>
  value(const std::map<std::string, at::Tensor> & inputs) const = 0;
  /// Evaluate the model and its full input Jacobian.
  virtual std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  jacobian(const std::map<std::string, at::Tensor> & inputs) const = 0;
  /// Evaluate the model and its parameter Jacobian (d output / d parameter).
  virtual std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  param_jacobian(const std::map<std::string, at::Tensor> & inputs) const = 0;
  /// Replace a (runtime-flexible) model parameter's value.
  virtual void set_parameter(const std::string & name, const at::Tensor & value) = 0;
  ///@}

  /// The compute device this rank's workload runs on (for cpp-aoti, the scheduler-assigned device).
  virtual at::Device device() const = 0;
};

/**
 * Handle wrapping the cpp-eager runtime (embeds CPython; loads from a source `.i`). Single-device:
 * the model is pinned to one device (cpp-eager has no scheduler / multi-device dispatch).
 */
class EagerModelHandle : public NEML2ModelHandle
{
public:
  EagerModelHandle(const std::string & input_file,
                   const std::string & model_name,
                   const at::Device & device,
                   const std::vector<std::string> & load)
    : _m(input_file, model_name, device, load)
  {
  }

  const std::vector<std::string> & input_names() const override { return _m.input_names(); }
  const std::vector<std::string> & output_names() const override { return _m.output_names(); }
  const std::vector<std::vector<int64_t>> & input_base_shapes() const override
  {
    return _m.input_base_shapes();
  }
  const std::vector<std::vector<int64_t>> & output_base_shapes() const override
  {
    return _m.output_base_shapes();
  }
  const std::map<std::string, std::vector<int64_t>> & parameter_base_shapes() const override
  {
    return _m.parameter_base_shapes();
  }

  std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  jacobian(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.jacobian(inputs);
  }
  std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  param_jacobian(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.param_jacobian(inputs);
  }
  void set_parameter(const std::string & name, const at::Tensor & value) override
  {
    _m.set_parameter(name, value);
  }
  std::map<std::string, at::Tensor>
  value(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.forward(inputs);
  }
  at::Device device() const override { return _m.device(); }

private:
  neml2::eager::Model _m;
};

/**
 * Handle wrapping the cpp-aoti runtime: a `neml2::aoti::DispatchedModel` backed by the
 * self-describing artifact folder that `neml2-compile` writes (a shared `metadata.json` plus
 * per-`<device>/<dtype>/` `.pt2` binaries). That folder carries everything the runtime needs --
 * structure, solver config, I/O names, promoted parameters -- so the `neml2-compile` stub `.i` is
 * no longer required (it was only a `name -> artifact_path` indirection).
 *
 * For a friendly UX the `input` is resolved flexibly (see ::load): the artifact folder itself, the
 * `metadata.json` inside it, or the legacy stub `.i` are all accepted. An `MPISimpleScheduler` pins
 * this rank's workload to a device, round-robining the device list over the MPI ranks on each node
 * (the single "cpu" entry sends every rank's local batch to CPU). This matches MOOSE's MPI domain
 * decomposition -- each rank evaluates its own local batch.
 */
class AOTIModelHandle : public NEML2ModelHandle
{
public:
  AOTIModelHandle(const std::string & input,
                  const std::string & model_name,
                  const std::vector<std::string> & devices,
                  const std::vector<std::size_t> & batch_sizes,
                  const void * comm)
    : _m(load(input, model_name, devices, batch_sizes, comm))
  {
  }

  const std::vector<std::string> & input_names() const override { return _m.input_names(); }
  const std::vector<std::string> & output_names() const override { return _m.output_names(); }
  const std::vector<std::vector<int64_t>> & input_base_shapes() const override
  {
    return _m.input_base_shapes();
  }
  const std::vector<std::vector<int64_t>> & output_base_shapes() const override
  {
    return _m.output_base_shapes();
  }
  const std::map<std::string, std::vector<int64_t>> & parameter_base_shapes() const override
  {
    return _m.parameter_base_shapes();
  }

  std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  jacobian(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.jacobian(inputs);
  }
  std::pair<std::map<std::string, at::Tensor>, neml2::aoti::VariablePairJacobian>
  param_jacobian(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.param_jacobian(inputs);
  }
  void set_parameter(const std::string & name, const at::Tensor & value) override
  {
    _m.set_parameter(name, value);
  }
  std::map<std::string, at::Tensor>
  value(const std::map<std::string, at::Tensor> & inputs) const override
  {
    return _m.forward(inputs);
  }
  at::Device device() const override { return _m.device(); }

private:
  /// Build the MPI scheduler (round-robin device list over the node's ranks) and load the dispatched
  /// aoti model. `input` is resolved flexibly for a friendlier UX:
  ///   - a directory                -> the artifact root, loaded directly (self-describing);
  ///   - a `metadata.json` file     -> its parent directory (metadata.json always sits at the root);
  ///   - anything else (a stub `.i`)-> parsed by `neml2::aoti::load_model`, which resolves the
  ///                                   recorded `artifact_path` (absolute, or relative to the stub).
  /// The folder and metadata.json forms are self-describing and ignore `model_name`; only the stub
  /// form selects a named model, so `model_name` is required there (as before). Prefer the folder:
  /// it is the relocatable unit -- the stub references its folder by path (increasingly a relative
  /// one in neml2), so moving the stub alone breaks that link, whereas the folder moves cleanly.
  static neml2::aoti::DispatchedModel load(const std::string & input,
                                           const std::string & model_name,
                                           const std::vector<std::string> & devices,
                                           const std::vector<std::size_t> & batch_sizes,
                                           const void * comm)
  {
    neml2::aoti::MPISimpleScheduler::Config config;
    config.devices = devices;
    config.batch_sizes = batch_sizes;
    config.comm = comm;
    auto scheduler = std::make_shared<neml2::aoti::MPISimpleScheduler>(config);

    const std::filesystem::path path(input);
    if (std::filesystem::is_directory(path))
      return neml2::aoti::DispatchedModel(path, scheduler);
    if (path.filename() == "metadata.json")
      return neml2::aoti::DispatchedModel(path.parent_path(), scheduler);
    return neml2::aoti::load_model(input, model_name, scheduler);
  }

  neml2::aoti::DispatchedModel _m;
};

/**
 * Construct a NEML2 model handle for the requested runtime.
 *
 * @param eager        When true, build a cpp-eager model from the source `.i`; when false, build a
 *                     cpp-aoti dispatched model from the compiled artifact.
 * @param input        For cpp-eager, the path to the source NEML2 `.i`. For cpp-aoti, any of: the
 *                     `neml2-compile` artifact folder, the `metadata.json` inside it, or the legacy
 *                     stub `.i` (see AOTIModelHandle::load).
 * @param model        Name of the model in the `.i`. Required for cpp-eager, and for cpp-aoti only
 *                     when `input` is a stub `.i`; the folder / metadata.json forms ignore it.
 * @param devices      Compute device(s). cpp-eager pins to the first; cpp-aoti dispatches over the
 *                     list (a CUDA list round-robins over the MPI ranks; a single CPU runs
 * locally).
 * @param batch_sizes  Per-device chunk size (0 = whole batch). Length 1 broadcasts to all devices.
 * @param comm         Pointer to the host MPI communicator (nullptr => MPI_COMM_WORLD); used by the
 *                     CUDA MPI scheduler only.
 * @param load         External Python extension paths to import before building (cpp-eager only).
 */
inline std::unique_ptr<NEML2ModelHandle>
makeNEML2ModelHandle(bool eager,
                     const std::string & input,
                     const std::string & model,
                     const std::vector<std::string> & devices,
                     const std::vector<std::size_t> & batch_sizes,
                     const void * comm,
                     const std::vector<std::string> & load)
{
  if (eager)
    return std::make_unique<EagerModelHandle>(input, model, at::Device(devices.at(0)), load);
  return std::make_unique<AOTIModelHandle>(input, model, devices, batch_sizes, comm);
}

#endif // NEML2_ENABLED
