#include "Kernel.h"

#ifndef DIFFUSION_H
#define DIFFUSION_H
//Forward Declarations
class Diffusion;

class Diffusion : public Kernel
{
public:

  Diffusion(Parameters parameters,
                 std::string var_name,
                 std::vector<std::string> coupled_to=std::vector<std::string>(0),
                 std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as)
  {}

protected:
  virtual Real computeQpResidual()
  {
    return _dphi[_i][_qp]*_grad_u[_qp];
  }

  virtual Real computeQpJacobian()
  {
    return _dphi[_i][_qp]*_dphi[_j][_qp];
  }

};
#endif //DIFFUSION_H
