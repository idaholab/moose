#include "Kernel.h"

#ifndef IMPLICITBD2
#define IMPLICITBD2

//Forward Declarations
class ImplicitBackwardDifference2;

template<>
Parameters valid_params<ImplicitBackwardDifference2>();

class ImplicitBackwardDifference2 : public Kernel
{
public:

  ImplicitBackwardDifference2(Parameters parameters,
                              std::string var_name,
                              std::vector<std::string> coupled_to=std::vector<std::string>(0),
                              std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as)
  { _t_scheme = 1;}

protected:
  virtual Real computeQpResidual()
  {
     if(_t_step==1) // First step, use BE
        return _phi[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
     else
        return _phi[_i][_qp]*((_bdf2_wei[2]*_u[_qp]+_bdf2_wei[1]*_u_old[_qp]+_bdf2_wei[0]*_u_older[_qp])/_dt);
  }

  virtual Real computeQpJacobian()
  {
     if(_t_step==1) // First step, use BE
       return _phi[_i][_qp]*_phi[_j][_qp]/_dt;
     else
       return _phi[_i][_qp]*_phi[_j][_qp]*_bdf2_wei[2]/_dt;
  }

};
#endif //IMPLICITBD2
