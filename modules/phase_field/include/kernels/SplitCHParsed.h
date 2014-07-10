#ifndef SPLITCHPARSED_H
#define SPLITCHPARSED_H

#include "SplitCHCRes.h"
#include "DerivativeKernelInterface.h"

//Forward Declarations
class SplitCHParsed;

template<>
InputParameters validParams<SplitCHParsed>();

/**
 * CHParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial.
 * This is the split operator variant.
 * \see CHParsed
 */
class SplitCHParsed : public DerivativeKernelInterface<SplitCHCRes>
{
public:
  SplitCHParsed(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);

private:
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
};

#endif // SPLITCHPARSED_H
