/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PARSEDMATERIALHELPER_H
#define PARSEDMATERIALHELPER_H

#include "FunctionMaterialBase.h"
#include "FunctionParserUtils.h"
#include "FunctionMaterialPropertyDescriptor.h"

#include "libmesh/fparser_ad.hh"

// forward declatration
class ParsedMaterialHelper;

template <>
InputParameters validParams<ParsedMaterialHelper>();

/**
 * Helper class to perform the parsing and optimization of the
 * function expression.
 */
class ParsedMaterialHelper : public FunctionMaterialBase, public FunctionParserUtils
{
public:
  enum VariableNameMappingMode
  {
    USE_MOOSE_NAMES,
    USE_PARAM_NAMES
  };

  ParsedMaterialHelper(const InputParameters & parameters, VariableNameMappingMode map_mode);

  void functionParse(const std::string & function_expression);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values);

protected:
  virtual void computeProperties();

  // tasks to perform after parsing the primary function
  virtual void functionsPostParse();

  // run FPOptimizer on the parsed function
  virtual void functionsOptimize();

  /// The undiffed free energy function parser object.
  ADFunctionPtr _func_F;

  /// variable names used in the expression (depends on the map_mode)
  std::vector<std::string> _variable_names;

  /// convenience typedef for the material property descriptors
  typedef std::vector<FunctionMaterialPropertyDescriptor> MatPropDescriptorList;

  /// Material property descriptors (obtained by parsing _mat_prop_expressions)
  MatPropDescriptorList _mat_prop_descriptors;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  /**
   * Flag to indicate if MOOSE nonlinear variable names should be used as FParser variable names.
   * This should be true only for DerivativeParsedMaterial. If set to false, this class looks up the
   * input parameter name for each coupled variable and uses it as the FParser parameter name when
   * parsing the FParser expression.
   */
  const VariableNameMappingMode _map_mode;
};

#endif // PARSEDMATERIALHELPER_H
