#ifndef CHBULKPFCTRAD_H
#define CHBULKPFCTRAD_H

#include "CHBulk.h"


//Forward Declarations
class CHBulkPFCTrad;

template<>
InputParameters validParams<CHBulkPFCTrad>();

class CHBulkPFCTrad : public CHBulk
{
public:

  CHBulkPFCTrad(const std::string & name, InputParameters parameters);
  
protected:

  virtual RealGradient computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c);

private:
  
  MaterialProperty<Real> & _C0;
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;
  
  
};
#endif //CHBULKPFCTRAD_H
