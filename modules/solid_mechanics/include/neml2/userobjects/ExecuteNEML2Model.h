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
#include "ElementUserObject.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(ExecuteNEML2Model, ElementUserObject);
#else

#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#include "neml2/models/Model.h"

#include "NEML2ModelInterface.h"
#include <map>

class MOOSEToNEML2;
class MOOSEToNEML2Parameter;

/**
 * ExecuteNEML2Model executes a NEML2 model. The NEML2 input variables are gathered by UserObjects
 * derived from MOOSEToNEML2. The execution of the NEML2 model happens in finalize(). This class is
 * derived from ElementUserObject and iterates over the mesh along with the MOOSEToNEML2 objects.
 * While iterating it generates a map from element ID to batch index which is later used when
 * retrieving the NEML2 model outputs.
 */
class ExecuteNEML2Model : public NEML2ModelInterface<ElementUserObject>
{
public:
  static InputParameters validParams();

  ExecuteNEML2Model(const InputParameters & params);

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

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
  /// Register a variable gathered by a gatherer UO
  virtual void addUOVariable(const UserObjectName & uo_name, const neml2::VariableName & uo_var);

  /// Prevent output and derivative retrieval after construction
  virtual void checkExecutionStage() const final;

  /// Set parameters from parameter UO and/or enable AD
  virtual void setParameter();

  /// Obtain derivative of output with respect to parameters
  virtual void getParameterDerivative();

  /// Determine whether the material model should be called
  virtual bool shouldCompute();

  /// Update the forces driving the material model update
  virtual void updateForces();

  /// Apply the predictor to set current trial state
  virtual void applyPredictor();

  /// Perform the material update
  virtual void solve();

  // set of variables to skip
  std::set<neml2::VariableName> _skip_vars;

  // set of provided and required inputs
  std::set<neml2::VariableName> _provided_inputs;

  /// MOOSE data gathering user objects
  std::vector<const MOOSEToNEML2 *> _gather_uos;                // for input
  std::vector<const MOOSEToNEML2Parameter *> _gather_param_uos; // for parameters

  /// (optional) NEML2 time input
  const neml2::VariableName _neml2_time;

  /// (optional) NEML2 old time input
  const neml2::VariableName _neml2_time_old;

  /// The input vector of the material model
  neml2::LabeledVector _in;

  /// The output vector of the material model
  neml2::LabeledVector _out;

  /// The derivative of the output vector w.r.t. the input vector
  neml2::LabeledMatrix _dout_din;

  /// Highest current batch index
  std::size_t _batch_index;

  /// Map from element IDs to batch indices
  std::map<dof_id_type, std::size_t> _elem_to_batch_index;

  /// cache the index for the current element
  mutable std::pair<dof_id_type, std::size_t> _elem_to_batch_index_cache;

  /// map from input variable names to corresponding MOOSE2NEML2 UOs
  std::map<neml2::VariableName, std::string> _var_to_uo;

  /// model outputs (the map is an unstable container and we're doling out references to the items. we must not change it after construction!)
  std::map<neml2::VariableName, neml2::Tensor> _outputs;

  /// model output derivatives (see above))
  std::map<std::pair<neml2::VariableName, std::string>, neml2::Tensor> _doutputs;

  /// model output derivatives wrt parameters (see above)
  std::map<std::pair<neml2::VariableName, std::string>, neml2::Tensor> _doutputs_dparams;

  /// flag that indicates if output data has been fully computed
  bool _output_ready;

  /// set of output variables that were retrieved
  mutable std::set<neml2::VariableName> _retrieved_outputs;

  /// set of derivatives that were retrieved
  mutable std::set<std::tuple<neml2::VariableName, neml2::VariableName, neml2::Tensor *>>
      _retrieved_derivatives;

  /// set of parameter derivatives that were retrieved
  mutable std::set<std::tuple<neml2::VariableName, std::string, neml2::Tensor *>>
      _retrieved_parameter_derivatives;
};

#endif
