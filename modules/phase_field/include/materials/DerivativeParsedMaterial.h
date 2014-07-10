#ifndef DERIVATIVEPARSEDMATERIAL_H
#define DERIVATIVEPARSEDMATERIAL_H

#include "DerivativeBaseMaterial.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class DerivativeParsedMaterial;

template<>
InputParameters validParams<DerivativeParsedMaterial>();

/**
 * DerivativeBaseMaterial child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class DerivativeParsedMaterial : public DerivativeBaseMaterial
{
public:
  DerivativeParsedMaterial(const std::string & name,
                    InputParameters parameters);

  virtual ~DerivativeParsedMaterial();

protected:
  virtual unsigned int expectedNumArgs();
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
  // std::vector<MaterialProperty<Real> *> _mat_prop;

  /// Array to stage the parameters passed to the functions when calling Eval.
  Real * _func_params;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  // Dummy implementations
  virtual Real computeF();
  virtual Real computeDF(unsigned int);
  virtual Real computeD2F(unsigned int, unsigned int);
};

#endif // DERIVATIVEPARSEDMATERIAL_H
