#ifndef DERIVATIVEPARSEDMATERIALHELPER_H
#define DERIVATIVEPARSEDMATERIALHELPER_H

#include "DerivativeBaseMaterial.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class DerivativeParsedMaterialHelper;

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>();

/**
 * Helper class to perform the bulk of the bulk of teh auto derivative taking.
 */
class DerivativeParsedMaterialHelper : public DerivativeBaseMaterial
{
public:
  DerivativeParsedMaterialHelper(const std::string & name,
                                 InputParameters parameters);

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

private:
  void functionsDerivative();
  void functionsOptimize();

  /// Shorthand for an autodiff function parser object.
  typedef FunctionParserADBase<Real> ADFunction;

  /// The undiffed free energy function parser object.
  ADFunction _func_F;

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

  // Dummy implementations
  virtual Real computeF();
  virtual Real computeDF(unsigned int);
  virtual Real computeD2F(unsigned int, unsigned int);
};

#endif // DERIVATIVEPARSEDMATERIALHELPER_H
