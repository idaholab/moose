#ifndef GRAVITYPOWER_H
#define GRAVITYPOWER_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class GravityPower;

template<>
InputParameters valid_params<GravityPower>();

class GravityPower : public Kernel
{
public:

  GravityPower(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
               std::vector<std::string> coupled_as=std::vector<std::string>(0));
  

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _pv_var;
  std::vector<Real> & _pv;

  Real _acceleration;
};
 
#endif
