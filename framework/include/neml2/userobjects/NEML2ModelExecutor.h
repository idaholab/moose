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

  /// Perform the material update
  virtual bool solve();

  /// Extract outputs and their derivatives with respect to the input variables
  virtual void extractOutputs();

  /// Save stateful variables for on-device state advance
  void advanceState();

  /// The NEML2BatchIndexGenerator used to generate the element-to-batch-index map
  const NEML2BatchIndexGenerator & _batch_index_generator;

  /// Advance state on device (rather than via MOOSE material properties)
  const bool _manage_state_advance;

  /// Dump input tensor info on failure to aid debugging
  const bool _debug_inputs_on_failure;

  /// flag that indicates if output data has been fully computed
  bool _output_ready;

  /// The input variables of the material model (name -> (batch, *base_shape) tensor)
  std::map<std::string, at::Tensor> _in;

  /// Model parameter values gathered from MOOSE (qualified name -> tensor), set on the model
  /// before each evaluation
  std::map<std::string, at::Tensor> _model_params;

  /// The output variables of the material model
  std::map<std::string, at::Tensor> _out;

  /// Cached stateful variables from the last successful step (for on-device advance)
  std::map<std::string, at::Tensor> _state_vars;

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
#endif
};
