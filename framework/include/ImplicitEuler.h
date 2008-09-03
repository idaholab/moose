#include "Kernel.h"

//Forward Declarations
class ImplicitEuler;

template<>
Parameters valid_params<ImplicitEuler>();

class ImplicitEuler : public Kernel
{
public:

  ImplicitEuler(Parameters parameters,
                 std::string var_name,
                 std::vector<std::string> coupled_to=std::vector<std::string>(0),
                 std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as)
  {}

protected:
  virtual Real computeQpResidual()
  {
    return _phi[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
  }

  virtual Real computeQpJacobian()
  {
    return _phi[_i][_qp]*_phi[_j][_qp]/_dt;
  }

};
