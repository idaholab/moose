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
#include "neml2/models/Model.h"
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

  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

protected:
  const NEML2ActionCommon & getCommonAction() const;

#ifdef NEML2_ENABLED

  const FileName & fname() const { return _fname; }

  struct VariableMapping
  {
    std::string name;
    NEML2Utils::MOOSEIOType moose_type;
    neml2::TensorType neml2_type;
    std::size_t history_order;
  };

  struct ParameterMapping
  {
    std::string name;
    NEML2Utils::MOOSEIOType moose_type;
    neml2::TensorType neml2_type;
  };

  struct DerivativeMapping
  {
    std::string name;
    std::string y;
    std::string x;
  };

  /// Set up MOOSE-NEML2 input variable mappings
  void setupInputMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 output variable mappings
  void setupOutputMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 model parameter mappings
  void setupParameterMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 derivative mappings
  void setupDerivativeMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 parameter derivative mappings
  void setupParameterDerivativeMappings(const neml2::Model &);

  /// Infer the MOOSE IO type from the variable name and type
  NEML2Utils::MOOSEIOType inferMOOSEIOType(const neml2::VariableName & name,
                                           const neml2::TensorType & type) const;

  /// Whether a MATERIAL input should be gathered from interface material data
  bool isInterfaceMaterialInput(const std::string & moose_name) const;

  /// Name of the NEML2 input file
  FileName _fname;

  /// List of cli-args
  std::vector<std::string> _cli_args;

  /// The neml2 model
  std::shared_ptr<neml2::Model> _model;

  /// MOOSE-NEML2 input variable mappings
  std::vector<VariableMapping> _inputs;

  /// MOOSE-NEML2 model parameter mappings
  std::vector<ParameterMapping> _params;

  /// MOOSE-NEML2 output variable mappings
  std::vector<VariableMapping> _outputs;

  /// MOOSE-NEML2 derivative mappings
  std::vector<DerivativeMapping> _derivs;

  /// MOOSE-NEML2 parameter derivative mappings
  std::vector<DerivativeMapping> _param_derivs;

#endif
  /// Name of the NEML2Executor user object
  const UserObjectName _executor_name;

  /// Name of the NEML2BatchIndexGenerator user object
  const UserObjectName _idx_generator_name;

  /// Blocks this sub-block action applies to
  const std::vector<SubdomainName> _block;

  /// Interfaces this sub-block action applies to
  const std::vector<BoundaryName> _interface;

  /// If true, only create the boundary (interface) material and batch interface side data
  const bool _interface_only;

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
