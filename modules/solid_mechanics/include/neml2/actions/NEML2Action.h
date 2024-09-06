//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#endif

#include "Action.h"

/**
 * Action to set up NEML2 objects.
 */
class NEML2Action : public Action
{
public:
  static InputParameters validParams();

  NEML2Action(const InputParameters &);

  virtual void act() override;

protected:
#ifdef NEML2_ENABLED

  enum class MOOSEIOType
  {
    MATERIAL,
    VARIABLE,
    POSTPROCESSOR
  };

  struct MOOSEIO
  {
    const std::string name;
    const MOOSEIOType type;
  };

  struct NEML2IO
  {
    const neml2::VariableName name;
    const neml2::TensorType type;
  };

  struct NEML2Param
  {
    const std::string name;
    const neml2::TensorType type;
  };

  struct VariableMapping
  {
    const MOOSEIO moose;
    const NEML2IO neml2;
  };

  struct ParameterMapping
  {
    const MOOSEIO moose;
    const NEML2Param neml2;
  };

  struct DerivativeMapping
  {
    const MOOSEIO moose;
    const struct NEML2Derivative
    {
      const NEML2IO y;
      const NEML2IO x;
    } neml2;
  };

  struct ParameterDerivativeMapping
  {
    const MOOSEIO moose;
    const struct NEML2Derivative
    {
      const NEML2IO y;
      const NEML2Param x;
    } neml2;
  };

  /// Set up MOOSE-NEML2 input variable mappings
  void setupInputMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 model parameter mappings
  void setupParameterMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 output variable mappings
  void setupOutputMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 derivative mappings
  void setupDerivativeMappings(const neml2::Model &);

  /// Set up MOOSE-NEML2 parameter derivative mappings
  void setupParameterDerivativeMappings(const neml2::Model &);

  /// MOOSE-NEML2 input variable mappings
  std::vector<VariableMapping> _inputs;

  /// MOOSE-NEML2 model parameter mappings
  std::vector<ParameterMapping> _params;

  /// MOOSE-NEML2 output variable mappings
  std::vector<VariableMapping> _outputs;

  /// MOOSE-NEML2 derivative mappings
  std::vector<DerivativeMapping> _derivs;

  /// MOOSE-NEML2 parameter derivative mappings
  std::vector<ParameterDerivativeMapping> _param_derivs;

#endif
  /// Whether to print additional information about the model
  bool _verbose;

  /// Name of the NEML2Executor user object
  const UserObjectName _executor_name;

  /// Name of the NEML2BatchIndexGenerator user object
  const UserObjectName _idx_generator_name;

  /// Blocks this sub-block action applies to
  const std::vector<SubdomainName> _block;

  /// Material property additional outputs
  std::map<MaterialPropertyName, std::vector<OutputName>> _export_output_targets;
};
