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
  const neml2::Tensor & getOutput(const neml2::VariableName & output_name) const;

  /// Get a reference(!) to the requested output derivative view
  const neml2::Tensor & getOutputDerivative(const neml2::VariableName & output_name,
                                            const neml2::VariableName & input_name) const;

  /// Get a reference(!) to the requested output parameter derivative view
  const neml2::Tensor & getOutputParameterDerivative(const neml2::VariableName & output_name,
                                                     const std::string & parameter_name) const;

  /**
   * Get a reference(!) to the requested output parameter derivative vector-Jacobian product view,
   * i.e. the contraction of the gathered cotangent of \p output_name with the derivative of
   * \p output_name with respect to the model parameter \p parameter_name. The contraction is
   * evaluated by a single reverse pass seeded with the cotangent, so the full derivative is never
   * materialized. The result carries the shape of the model parameter, i.e. one scalar per
   * quadrature point for a scalar-valued model parameter.
   */
  const neml2::Tensor & getOutputParameterVJP(const neml2::VariableName & output_name,
                                              const std::string & parameter_name) const;

  /// check if the output is fully computed and ready to be fetched
  bool outputReady() const { return _output_ready; }

protected:
  /// Register a NEML2 input variable gathered by a gatherer
  virtual void addGatheredVariable(const UserObjectName &, const neml2::VariableName &);

  /// Register a NEML2 model parameter gathered by a gatherer
  virtual void addGatheredParameter(const UserObjectName &, const std::string &);

  /**
   * Register a reverse-mode cotangent gathered by a gatherer. The NEML2 name of the gatherer
   * identifies the NEML2 output variable the cotangent is paired with.
   */
  virtual void addGatheredCotangent(const UserObjectName &, const MOOSEToNEML2 &);

  /// Prevent output and derivative retrieval after construction
  virtual void checkExecutionStage() const final;

  /// Fill input variables and model parameters using the gatherers
  virtual void fillInputs();

  /// Perform the material update
  virtual bool solve();

  /**
   * Extract output derivatives with respect to input variables and model parameters, as well as
   * the vector-Jacobian products between the gathered cotangents and the parameter derivatives
   */
  virtual void extractOutputs();

  /// Expand tensor shapes if necessary to conformal sizes
  virtual void expandInputs();

  /// Save stateful variables for on-device state advance
  void advanceState();

  /// The NEML2BatchIndexGenerator used to generate the element-to-batch-index map
  const NEML2BatchIndexGenerator & _batch_index_generator;

  /// Advance state on device (rather than via MOSOE material properties)
  const bool _manage_state_advance;

  /// Dump input tensor info on failure to aid debugging
  const bool _debug_inputs_on_failure;

  /// flag that indicates if output data has been fully computed
  bool _output_ready;

  /// The model parameters to update (gathered from MOOSE)
  std::map<std::string, neml2::Tensor> _model_params;

  /// The input variables of the material model
  neml2::ValueMap _in;

  /// The output variables of the material model
  neml2::ValueMap _out;

  /// Cached variables from the last successful step (for on-device advance)
  neml2::ValueMap _state_vars;

  /// The derivative of the output variables w.r.t. the input variables
  neml2::DerivMap _dout_din;

  // set of gathered NEML2 input variables
  std::set<neml2::VariableName> _gathered_variable_names;

  // set of gathered NEML2 model parameters
  std::set<std::string> _gathered_parameter_names;

  /// MOOSE data gathering user objects
  std::vector<const MOOSEToNEML2 *> _gatherers;
  std::vector<const MOOSEToNEML2 *> _param_gatherers;

  /**
   * MOOSE data gathering user objects supplying reverse-mode cotangents, keyed by the NEML2 output
   * variable each cotangent is paired with. A cotangent is neither a model input variable nor a
   * model parameter, so it is never inserted into the model. It is read directly through
   * MOOSEToNEML2::gatheredData().
   */
  std::map<neml2::VariableName, const MOOSEToNEML2 *> _cotangent_gatherers;

  /// set of output variables that were retrieved (by other objects)
  mutable neml2::ValueMap _retrieved_outputs;

  /// set of derivatives that were retrieved (by other objects)
  mutable neml2::DerivMap _retrieved_derivatives;

  /// set of parameter derivatives that were retrieved (by other objects)
  mutable std::map<neml2::VariableName, std::map<std::string, neml2::Tensor>>
      _retrieved_parameter_derivatives;

  /// set of parameter derivative vector-Jacobian products that were retrieved (by other objects)
  mutable std::map<neml2::VariableName, std::map<std::string, neml2::Tensor>>
      _retrieved_parameter_vjps;

private:
  /// Whether an error was encountered
  bool _error;
  /// Error message
  std::string _error_message;
#endif
};
