#ifndef PARSEDMATERIALHELPER_H
#define PARSEDMATERIALHELPER_H

#include "FunctionMaterialBase.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class ParsedMaterialHelper;

template<>
InputParameters validParams<ParsedMaterialHelper>();

/**
 * Helper class to perform the parsing and optimization of the function expression
 */
class ParsedMaterialHelper : public virtual FunctionMaterialBase
{
public:
  enum VariableNameMappingMode {
    USE_MOOSE_NAMES, USE_PARAM_NAMES
  };

  ParsedMaterialHelper(const std::string & name,
                       InputParameters parameters,
                       VariableNameMappingMode map_mode = USE_PARAM_NAMES);

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

  // run FPOptimizer on the parsed function
  virtual void functionsOptimize();

  /// Shorthand for an autodiff function parser object.
  typedef FunctionParserADBase<Real> ADFunction;

  /// Evaluate FParser object and check EvalError
  Real evaluate(ADFunction *);

  /// The undiffed free energy function parser object.
  ADFunction * _func_F;

  /// Material properties needed by this free energy
  std::vector<MaterialProperty<Real> *> _mat_props;
  unsigned int _nmat_props;

  /// Array to stage the parameters passed to the functions when calling Eval.
  Real * _func_params;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  /// feature flags
  bool _enable_jit;
  bool _disable_fpoptimizer;
  bool _fail_on_evalerror;

  /**
   * Flag to indicate if MOOSE nonlinear variable names should be used as FParser variable names.
   * This should be true only for DerivativeParsedMaterial. If set to false, this class looks up the
   * input parameter name for each coupled variable and uses it as the FParser parameter name when
   * parsing the FParser expression.
   */
  const VariableNameMappingMode _map_mode;

  /// appropriate not a number value to return
  const Real _nan;

  /// table of FParser eval error codes
  static const char * _eval_error_msg[];
};

#endif //PARSEDMATERIALHELPER_H
