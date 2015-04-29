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
#include "libmesh/fparser_ad.hh"

// forward declatration
class ParsedMaterialHelper;

template<>
InputParameters validParams<ParsedMaterialHelper>();

/**
 * Helper class to perform the parsing and optimization of the
 * function expression.
 */
class ParsedMaterialHelper :
  public FunctionMaterialBase,
  public FunctionParserUtils
{
public:
  enum VariableNameMappingMode {
    USE_MOOSE_NAMES, USE_PARAM_NAMES
  };

  ParsedMaterialHelper(const std::string & name,
                       InputParameters parameters,
                       VariableNameMappingMode map_mode);

  virtual ~ParsedMaterialHelper();

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
  ADFunction * _func_F;

  /*
   * Material properties get fully described using this structure, including their dependent
   * variables and derivation state.
   */
  class FunctionMaterialPropertyDescriptor;

  /// Material property descriptors (obtained by parsing _mat_prop_expressions)
  std::vector<FunctionMaterialPropertyDescriptor> _mat_prop_descriptors;

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

class ParsedMaterialHelper::FunctionMaterialPropertyDescriptor
{
public:
  /*
   * The descriptor is constructed with an expression that describes the
   * material property.
   * Examples:
   *   'F'               A material property called 'F' with no declared variable
   *                     dependencies (i.e. vanishing derivatives)
   *   'F(c,phi)'        A material property called 'F' with declared dependence
   *                     on 'c' and 'phi' (uses DerivativeFunctionMaterial rules to
   *                     look up the derivatives)
   *   'a:=D[x(t),t,t]'  The second time derivative of the t-dependent material property 'x'
   *                     which will be referred to as 'a' in the function expression.
   */
  FunctionMaterialPropertyDescriptor(const std::string &, ParsedMaterialHelper *);

  /// default constructor
  FunctionMaterialPropertyDescriptor();

  /// copy constructor
  FunctionMaterialPropertyDescriptor(const FunctionMaterialPropertyDescriptor &);

  /// get the fparser symbol name
  const std::string & getSymbolName() const { return _fparser_name; };

  /// get the property name
  const std::string getPropertyName() const
  {
    mooseAssert( _parent_material != NULL, "_parent_material pointer is NULL" );
    return _parent_material->propertyName(_base_name, _derivative_vars);
  };

  /// get the property reference
  const MaterialProperty<Real> & value() const
  {
    mooseAssert( _value != NULL, "_value pointer is NULL" );
    return *_value;
  }

private:
  void parseDerivative(const std::string &);
  void parseDependentVariables(const std::string &);

  /// name used in function expression
  std::string _fparser_name;

  /// function material property base name
  std::string _base_name;

  std::vector<std::string> _dependent_vars;
  std::vector<std::string> _derivative_vars;

  /// material property value
  MaterialProperty<Real> * _value;

  /// parent material class
  ParsedMaterialHelper * _parent_material;
};

#endif //PARSEDMATERIALHELPER_H
