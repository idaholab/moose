#ifndef GRAVITYFORCE_H
#define GRAVITYFORCE_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class GravityForce;

template<>
Parameters valid_params<GravityForce>();

class GravityForce : public Kernel
{
public:

  GravityForce(std::string name,
                  Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
               std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _p_var;
  std::vector<Real> & _p;

  Real _acceleration;
};
 
#endif
