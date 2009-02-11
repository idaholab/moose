#include "Kernel.h"

#ifndef COUPLEDFORCE_H
#define COUPLEDFORCE_H
//Forward Declarations
class CoupledForce;

/**
 * Simple class to demonstrate off diagonal Jacobian contributijons.
 */
class CoupledForce : public Kernel
{
public:

  CoupledForce(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to=std::vector<std::string>(0),
            std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _v_var(coupled("v")),
    _v(coupledVal("v"))
  {}

protected:
  virtual Real computeQpResidual()
  {
    return -_v[_qp]*_phi[_i][_qp];
  }

  virtual Real computeQpJacobian()
  {
    return 0;
  }

  virtual Real computeQpOffDiagJacobian(unsigned int jvar)
  {
    if(jvar == _v_var)
      return -_phi[_j][_qp]*_phi[_i][_qp];    
    
    return 0.0;
  }

private:
  unsigned int _v_var;
  std::vector<Real> & _v;
};
#endif //COUPLEDFORCE_H
