#ifndef CHPARSED_H
#define CHPARSED_H

#include "CHBulk.h"
#include "DerivativeKernelInterface.h"

//Forward Declarations
class CHParsed;

template<>
InputParameters validParams<CHParsed>();

/**
 * CHParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial.
 * \see SplitCHParsed
 */
class CHParsed : public DerivativeKernelInterface<CHBulk>
{
public:
  CHParsed(const std::string & name, InputParameters parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);

private:
  std::vector<MaterialProperty<Real>* > _second_derivatives;
  std::vector<MaterialProperty<Real>* > _third_derivatives;
  std::vector<VariableGradient *> _grad_vars;
};

#endif // CHPARSED_H
