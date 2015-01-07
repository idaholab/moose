#ifndef DERIVATIVEPARSEDMATERIALHELPER_H
#define DERIVATIVEPARSEDMATERIALHELPER_H

#include "DerivativeBaseMaterial.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class DerivativeParsedMaterialHelper;

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>();

/**
 * Helper class to perform the bulk of the bulk of the auto derivative taking.
 */
class DerivativeParsedMaterialHelper : public DerivativeBaseMaterial
{
public:
  enum VariableNameMappingMode {
    USE_MOOSE_NAMES, USE_PARAM_NAMES
  };

  DerivativeParsedMaterialHelper(const std::string & name,
                                 InputParameters parameters,
                                 VariableNameMappingMode map_mode = USE_PARAM_NAMES);

  virtual ~DerivativeParsedMaterialHelper();

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

  void functionsDerivative();
  void functionsOptimize();

  /// Shorthand for an autodiff function parser object.
  typedef FunctionParserADBase<Real> ADFunction;

  /// Evaluate FParser object and check EvalError
  Real evaluate(ADFunction *);

  /// The undiffed free energy function parser object.
  ADFunction * _func_F;

  /// The first derivatives of the free energy (function parser objects).
  std::vector<ADFunction *> _func_dF;

  /// The second derivatives of the free energy (function parser objects).
  std::vector<std::vector<ADFunction *> > _func_d2F;

  /// The third derivatives of the free energy (function parser objects).
  std::vector<std::vector<std::vector<ADFunction *> > > _func_d3F;

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

#endif // DERIVATIVEPARSEDMATERIALHELPER_H
