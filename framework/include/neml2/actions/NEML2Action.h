//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <memory>

#ifdef NEML2_ENABLED
#include "NEML2ModelHandle.h"
#endif

#include "Action.h"
#include "NEML2Utils.h"
#include "DerivativeMaterialPropertyNameInterface.h"

class NEML2ActionCommon;

/**
 * Action to set up NEML2 objects.
 */
class NEML2Action : public Action, public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  NEML2Action(const InputParameters &);

  virtual void act() override;

protected:
  const NEML2ActionCommon & getCommonAction() const;

#ifdef NEML2_ENABLED

  const FileName & fname() const { return _fname; }

  struct VariableMapping
  {
    std::string name; ///< MOOSE quantity name (base name, no ~N lag suffix)
    NEML2Utils::MOOSEIOType moose_type;
    std::string moose_tensor_type; ///< MOOSE C++ tensor type string (e.g. SymmetricRankTwoTensor)
    std::size_t history_order;
  };

  struct DerivativeMapping
  {
    std::string name;
    std::string y;
    std::string x;
    std::string moose_tensor_type; ///< MOOSE C++ tensor type string of the derivative block
  };

  struct ParameterMapping
  {
    std::string moose_name; ///< MOOSE quantity name (from_moose source)
    std::string neml2_name; ///< fully-qualified NEML2 parameter name (set target)
    NEML2Utils::MOOSEIOType moose_type;
    std::string moose_tensor_type; ///< MOOSE C++ tensor type string
  };

  /// Set up MOOSE-NEML2 input variable mappings
  void setupInputMappings(const NEML2ModelHandle &);

  /// Set up MOOSE-NEML2 output variable mappings
  void setupOutputMappings(const NEML2ModelHandle &);

  /// Set up MOOSE-NEML2 derivative mappings
  void setupDerivativeMappings(const NEML2ModelHandle &);

  /// Set up MOOSE-NEML2 model parameter (value) mappings
  void setupParameterMappings(const NEML2ModelHandle &);

  /// Set up MOOSE-NEML2 output-parameter-derivative mappings
  void setupParameterDerivativeMappings(const NEML2ModelHandle &);

  /// Validate consistency between the action configuration and the model's I/O before MOOSE wires
  /// up (stateful) material properties. Guards against two configurations that otherwise crash
  /// during stateful material property initialization: (1) 'initialize_outputs' naming a variable
  /// that is not a model output, and (2) an old (stateful) material input 'var~N' whose base 'var'
  /// is neither a current model input nor a model output (so nothing declares the property whose
  /// old value is requested).
  void checkStatefulConsistency(const NEML2ModelHandle &) const;

  /// Resolve a user-written parameter name to a fully-qualified NEML2 parameter name. Accepts an
  /// exact match or an unambiguous trailing ".<name>" suffix of a registered parameter. The
  /// candidates are the keys of the model's parameter_base_shapes() map.
  std::string
  resolveParameterName(const std::string & user_name,
                       const std::map<std::string, std::vector<int64_t>> & param_shapes) const;

  /// Infer the MOOSE IO type from the variable name (only scalar-typed variables can map to
  /// time/scalar/function/field quantities; everything else is a material property)
  NEML2Utils::MOOSEIOType inferMOOSEIOType(const std::string & name, bool is_scalar) const;

  /// Name of the NEML2 input file
  FileName _fname;

  /// List of cli-args
  std::vector<std::string> _cli_args;

  /// The neml2 model handle, used here only for introspection during setup. Built from the
  /// cpp-eager source '.i' or the cpp-aoti artifact folder -- both given by 'input' -- per 'eager'.
  std::unique_ptr<NEML2ModelHandle> _model;

  /// MOOSE-NEML2 input variable mappings
  std::vector<VariableMapping> _inputs;

  /// MOOSE-NEML2 output variable mappings
  std::vector<VariableMapping> _outputs;

  /// MOOSE-NEML2 derivative mappings
  std::vector<DerivativeMapping> _derivs;

  /// MOOSE-NEML2 model parameter (value) mappings
  std::vector<ParameterMapping> _params;

  /// MOOSE-NEML2 output-parameter-derivative mappings
  std::vector<DerivativeMapping> _param_derivs;

#endif
  /// Name of the NEML2Executor user object
  const UserObjectName _executor_name;

  /// Name of the NEML2BatchIndexGenerator user object
  const UserObjectName _idx_generator_name;

  /// Blocks this sub-block action applies to
  const std::vector<SubdomainName> _block;

  /// Input variables to skip (i.e., not to set up mappings for)
  std::vector<std::string> _skip_input_variables;

  /// Material property initial conditions
  std::map<MaterialPropertyName, MaterialPropertyName> _initialize_output_values;

  /// Material property additional outputs
  std::map<MaterialPropertyName, std::vector<OutputName>> _export_output_targets;

private:
#ifdef NEML2_ENABLED
  /// Get parameter lists for mapping between MOOSE and NEML2 quantities
  template <typename EnumType, typename T>
  std::tuple<std::vector<EnumType>, std::vector<T>>
  getInputParameterMapping(const std::string & source_opt, const std::string & name_opt) const
  {
    const auto moose_types = getParam<MultiMooseEnum>(source_opt).getSetValueIDs<EnumType>();
    const auto neml2_names = getParam<std::vector<T>>(name_opt);

    if (moose_types.size() != neml2_names.size())
      paramError(source_opt, source_opt, " must have the same length as ", name_opt);

    return {moose_types, neml2_names};
  }

  /// Print a summary of the NEML2 model
  void printSummary() const;
#endif

  /// Get the maximum length of all MOOSE names (for printing purposes)
  std::size_t getLongestMOOSEName() const;
};
