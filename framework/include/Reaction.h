#include "Kernel.h"

#ifndef REACTION_H
#define REACTION_H

//Forward Declarations
class Reaction;

class Reaction : public Kernel
{
public:

  Reaction(Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                  std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as)
  {}
  
protected:
  virtual Real computeQpResidual()
  {
    return _phi[_i][_qp]*_u[_qp];
  }
  virtual Real computeQpJacobian()
  {
    return _phi[_i][_qp]*_phi[_j][_qp];
  }
};
#endif //REACTION_H
