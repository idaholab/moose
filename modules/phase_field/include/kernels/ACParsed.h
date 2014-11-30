#ifndef ACPARSED_H
#define ACPARSED_H

#include "ACBulk.h"
#include "DerivativeKernelInterface.h"

//Forward Declarations
class ACParsed;

template<>
InputParameters validParams<ACParsed>();

/**
 * ACParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to computer the
 * residual for the bulk part of the Allen-Cahn equation.
 * \see ACParsed
 * \see SplitACParsed
 */
class ACParsed : public DerivativeKernelInterface<ACBulk>
{
public:
  ACParsed(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);

private:
  const MaterialProperty<Real> & _dFdEta;
  const MaterialProperty<Real> & _d2FdEta2;
};

#endif // ACPARSED_H
