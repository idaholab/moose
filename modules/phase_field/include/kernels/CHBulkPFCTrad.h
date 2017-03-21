#ifndef CHBULKPFCTRAD_H
#define CHBULKPFCTRAD_H

#include "CHBulk.h"

// Forward Declarations
class CHBulkPFCTrad;

template <>
InputParameters validParams<CHBulkPFCTrad>();

class CHBulkPFCTrad : public CHBulk<Real>
{
public:
  CHBulkPFCTrad(const InputParameters & parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);

private:
  const MaterialProperty<Real> & _C0;
  const MaterialProperty<Real> & _a;
  const MaterialProperty<Real> & _b;
};

#endif // CHBULKPFCTRAD_H
