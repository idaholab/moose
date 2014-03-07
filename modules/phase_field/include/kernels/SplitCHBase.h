#ifndef SPLITCHBase_H
#define SPLITCHBase_H

#include "Kernel.h"


//Forward Declarations
class SplitCHBase;

template<>
InputParameters validParams<SplitCHBase>();

class SplitCHBase : public Kernel
{
public:

  SplitCHBase(const std::string & name, InputParameters parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeDEDC(PFFunctionType type);

private:

};
#endif //SPLITCHBase_H
