#ifndef CHMath_H
#define CHMath_H

#include "CHBulk.h"

//Forward Declarations
class CHMath;

template<>
InputParameters validParams<CHMath>();

class CHMath : public CHBulk
{
public:

  CHMath(const std::string & name, InputParameters parameters);
  
protected:
  
  virtual RealGradient computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c);

private:
  
};
#endif //CHMath_H
