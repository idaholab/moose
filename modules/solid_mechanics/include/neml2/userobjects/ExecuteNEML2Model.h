//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2ModelInterface.h"
#include "ElementUserObject.h"

#include <map>

#ifdef NEML2_ENABLED
#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#include "neml2/models/Model.h"
#endif

class MOOSEToNEML2;

/**
 * ExecuteNEML2Model executes a NEML2 model. The NEML2 input variables and model parameters are
 * gathered by UserObjects derived from MOOSEToNEML2. The execution of the NEML2 model happens in
 * finalize(). This class is derived from ElementUserObject and iterates over the mesh along with
 * the MOOSEToNEML2Batched objects. While iterating it generates a map from element ID to batch
 * index which is later used when retrieving the NEML2 model outputs.
 */
class ExecuteNEML2Model : public NEML2ModelInterface<ElementUserObject>
{
public:
  /// Parameters that can be specified under the NEML2Action common area
  static InputParameters actionParams();

  static InputParameters validParams();

  ExecuteNEML2Model(const InputParameters & params);

#ifndef NEML2_ENABLED
  void initialize() override {}
  void execute() override {}
  void threadJoin(const UserObject &) override {}
  void finalize() override {}
#else
  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject &) override;
  void finalize() override;

  void initialSetup() override;

  /// Get the batch index for the given element ID
  std::size_t getBatchIndex(dof_id_type elem_id) const;

  /// Get a reference(!) to the requested output view
  const neml2::Tensor & getOutputView(const neml2::VariableName & output_name) const;

  /// Get a reference(!) to the requested output derivative view
  const neml2::Tensor & getOutputDerivativeView(const neml2::VariableName & output_name,
                                                const neml2::VariableName & input_name) const;

  /// Get a reference(!) to the requested output parameter derivative view
  const neml2::Tensor & getOutputParameterDerivativeView(const neml2::VariableName & output_name,
                                                         const std::string & parameter_name) const;

  /// check if the output is fully computed and ready to be fetched
  bool outputReady() const { return _output_ready; }

protected:
  /// Register a NEML2 input variable gathered by a gatherer
  virtual void addGatheredVariable(const UserObjectName &, const neml2::VariableName &);

  /// Register a NEML2 model parameter gathered by a gatherer
  virtual void addGatheredParameter(const UserObjectName &, const std::string &);

  /// Prevent output and derivative retrieval after construction
  virtual void checkExecutionStage() const final;

  /// Determine whether the material model should be called
  virtual bool shouldCompute();

  /// Fill input variables and model parameters using the gatherers
  virtual void fillInputs();

  /// Apply the predictor to set current trial state
  virtual void applyPredictor();

  /// Perform the material update
  virtual void solve();

  /// Extract output derivatives with respect to input variables and model parameters
  virtual void extractOutputs();

  /// flag that indicates if output data has been fully computed
  bool _output_ready;

  /// The input vector of the material model
  const neml2::LabeledVector & _in;

  /// The output vector of the material model
  const neml2::LabeledVector & _out;

  /// The derivative of the output vector w.r.t. the input vector
  const neml2::LabeledMatrix & _dout_din;

  // set of variables to skip
  std::set<neml2::VariableName> _skip_vars;

  // set of gathered NEML2 input variables
  std::set<neml2::VariableName> _gathered_variable_names;

  // set of gathered NEML2 model parameters
  std::set<std::string> _gathered_parameter_names;

  /// MOOSE data gathering user objects
  std::vector<const MOOSEToNEML2 *> _gatherers;

  /// Highest current batch index
  std::size_t _batch_index;

  /// Map from element IDs to batch indices
  std::map<dof_id_type, std::size_t> _elem_to_batch_index;

  /// cache the index for the current element
  mutable std::pair<dof_id_type, std::size_t> _elem_to_batch_index_cache;

  /// set of output variables that were retrieved (by other objects)
  mutable std::map<neml2::VariableName, neml2::Tensor> _retrieved_outputs;

  /// set of derivatives that were retrieved (by other objects)
  mutable std::map<std::pair<neml2::VariableName, neml2::VariableName>, neml2::Tensor>
      _retrieved_derivatives;

  /// set of parameter derivatives that were retrieved (by other objects)
  mutable std::map<std::pair<neml2::VariableName, std::string>, neml2::Tensor>
      _retrieved_parameter_derivatives;
#endif
};
