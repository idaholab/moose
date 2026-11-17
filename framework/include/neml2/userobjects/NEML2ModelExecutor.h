//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2ModelInterface.h"
#include "GeneralUserObject.h"
#include "NEML2BatchIndexGenerator.h"

class MOOSEToNEML2;

/**
 * NEML2ModelExecutor executes a NEML2 model. The NEML2 input variables and model parameters are
 * gathered by UserObjects derived from MOOSEToNEML2. This class is derived from GeneralUserObject
 * and is not threaded. It relies on a NEML2BatchIndexGenerator to generate batch indices.
 */
class NEML2ModelExecutor : public NEML2ModelInterface<GeneralUserObject>
{
public:
  /// Parameters that can be specified under the NEML2Action common area
  static InputParameters actionParams();

  static InputParameters validParams();

  NEML2ModelExecutor(const InputParameters & params);

#ifndef NEML2_ENABLED
  void initialize() override {}
  void execute() override {}
  void finalize() override {}
#else
  void initialize() override;
  void meshChanged() override;
  void execute() override;
  void finalize() override;

  void initialSetup() override;

  /// Get the batch index for the given element ID
  std::size_t getBatchIndex(dof_id_type elem_id) const;

  /// Get a reference(!) to the requested output view
  const at::Tensor & getOutput(const std::string & output_name) const;

  /// Get a reference(!) to the requested output derivative view
  const at::Tensor & getOutputDerivative(const std::string & output_name,
                                         const std::string & input_name) const;

  /// Get a reference(!) to the requested output-parameter-derivative view
  const at::Tensor & getOutputParameterDerivative(const std::string & output_name,
                                                  const std::string & parameter_name) const;

  /// check if the output is fully computed and ready to be fetched
  bool outputReady() const { return _output_ready; }

protected:
  /// Register a NEML2 input variable gathered by a gatherer
  virtual void addGatheredVariable(const UserObjectName &, const std::string &);

  /// Register a NEML2 model parameter gathered by a gatherer
  virtual void addGatheredParameter(const UserObjectName &, const std::string &);

  /// Prevent output and derivative retrieval after construction
  virtual void checkExecutionStage() const final;

  /// Fill input variables using the gatherers
  virtual void fillInputs();

  /// Perform the material update. Evaluates the value only, or the value and its derivatives when
  /// \p compute_derivative is true.
  virtual bool solve(bool compute_derivative);

  /// Extract outputs, and their derivatives with respect to the input variables when
  /// \p compute_derivative is true.
  virtual void extractOutputs(bool compute_derivative);

  /// Build the dump-file name for a failed constitutive update: '<model>_count<count>_rank<rank>.pt'.
  /// The count is advanced collectively so it is identical on every rank for a given failed update;
  /// passing rank_token = "*" therefore yields the glob the console message points users to (one file
  /// per failing rank). Step / execute-on / nonlinear-iteration context is reported in the message
  /// instead -- with a persistent counter they are redundant for uniquely identifying the file.
  std::string failedInputDumpName(unsigned int count, const std::string & rank_token) const;

  /// Block until all asynchronous device (CUDA) work has completed. On CUDA, NEML2 kernels are
  /// launched asynchronously, so a TIME_SECTION would otherwise attribute their time to whatever
  /// later forces an implicit sync (e.g. the output D2H copy) rather than to the phase that
  /// launched them. Calling this at phase boundaries makes the per-phase timings fair. No-op on CPU.
  void deviceSynchronize();

  /// Save stateful variables for on-device state advance
  void advanceState();

  /// The NEML2BatchIndexGenerator used to generate the element-to-batch-index map
  const NEML2BatchIndexGenerator & _batch_index_generator;

  /// Advance state on device (rather than via MOOSE material properties)
  const bool _manage_state_advance;

  /// Dump the NEML2 input tensors to a per-rank TorchScript file on solve failure, for offline debugging
  const bool _dump_inputs_on_failure;

  /// flag that indicates if output data has been fully computed
  bool _output_ready;

  /// Whether the retained input-derivative tensor is consistent with the current batch. The input
  /// Jacobian is recomputed only on Jacobian evaluations (see execute()), so a pure residual sweep
  /// reuses the retained derivative -- which is only safe while the batch is unchanged. Anything that
  /// resizes the batch (e.g. meshChanged() regenerating the element-to-batch-index map on element
  /// activation) invalidates it; execute() then forces one derivative recompute before it is read, so
  /// a retriever never indexes a stale, wrongly-sized derivative.
  bool _derivative_valid;

  /// The input variables of the material model (name -> (batch, *base_shape) tensor)
  std::map<std::string, at::Tensor> _in;

  /// Model parameter values gathered from MOOSE (qualified name -> tensor), set on the model
  /// before each evaluation
  std::map<std::string, at::Tensor> _model_params;

  /// The output variables of the material model
  std::map<std::string, at::Tensor> _out;

  /// Cached stateful variables from the last successful step (for on-device advance)
  std::map<std::string, at::Tensor> _state_vars;

  /// Base-shaped zero tensors for cold-started in-place-updated unknowns (an order-0 NEML2 input
  /// that is also a model output and is not gathered; see initialSetup). Injected into _in on every
  /// evaluation and broadcast over the batch by NEML2 (never advanced), so the return-map initial
  /// guess is 0 each step rather than the stale previous output value.
  std::map<std::string, at::Tensor> _zero_seed_vars;

  /// The derivative of the output variables w.r.t. the input variables: J[output][input]
  std::map<std::string, std::map<std::string, at::Tensor>> _dout_din;

  /// The derivative of the output variables w.r.t. the model parameters: P[output][parameter]
  std::map<std::string, std::map<std::string, at::Tensor>> _dout_dparam;

  // set of gathered NEML2 input variables
  std::set<std::string> _gathered_variable_names;

  // set of gathered NEML2 model parameters
  std::set<std::string> _gathered_parameter_names;

  /// MOOSE data gathering user objects (input variables)
  std::vector<const MOOSEToNEML2 *> _gatherers;

  /// MOOSE data gathering user objects (model parameters)
  std::vector<const MOOSEToNEML2 *> _param_gatherers;

  /// set of output variables that were retrieved (by other objects)
  mutable std::map<std::string, at::Tensor> _retrieved_outputs;

  /// set of derivatives that were retrieved (by other objects)
  mutable std::map<std::string, std::map<std::string, at::Tensor>> _retrieved_derivatives;

  /// set of output-parameter-derivatives that were retrieved (by other objects)
  mutable std::map<std::string, std::map<std::string, at::Tensor>> _retrieved_parameter_derivatives;

private:
  /// Whether an error was encountered
  bool _error;
  /// Error message
  std::string _error_message;
  /// Running count of failed constitutive updates dumped on this rank (disambiguates repeated dumps)
  unsigned int _num_failed_dumps;
#endif
};
