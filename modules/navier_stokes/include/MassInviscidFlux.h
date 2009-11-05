#ifndef MASSINVISCIDFLUX_H
#define MASSINVISCIDFLUX_H

#include "Kernel.h"


//ForwardDeclarations
class MassInviscidFlux;

template<>
Parameters valid_params<MassInviscidFlux>();

class MassInviscidFlux : public Kernel
{
public:

  MassInviscidFlux(std::string name,
                  Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                   std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _pu_var;
  std::vector<Real> & _pu;

  unsigned int _pv_var;
  std::vector<Real> & _pv;

  unsigned int _pw_var;
  std::vector<Real> & _pw;
};
 
#endif
