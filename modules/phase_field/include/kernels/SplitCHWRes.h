#ifndef SPLITCHWRes_H
#define SPLITCHWRes_H

#include "Kernel.h"


//Forward Declarations
class SplitCHWRes;

template<>
InputParameters validParams<SplitCHWRes>();

class SplitCHWRes : public Kernel
{
public:

  SplitCHWRes(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
private:
  std::string _mob_name;
  MaterialProperty<Real> & _mob;
  unsigned int _c_var;
  VariableValue & _c;
};
#endif //SPLITCHWRes_H
