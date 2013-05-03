#ifndef CHCpldPFCTrad_H
#define CHCpldPFCTrad_H

#include "CHSplitVar.h"

//Forward Declarations
class CHCpldPFCTrad;

template<>
InputParameters validParams<CHCpldPFCTrad>();

class CHCpldPFCTrad : public CHSplitVar
{
public:

  CHCpldPFCTrad(const std::string & name, InputParameters parameters);
  
protected:
  virtual RealGradient precomputeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  

private:
  std::string _coeff_name;
  MaterialProperty<Real> & _coeff;

  
};
#endif //CHCpldPFCTrad_H
