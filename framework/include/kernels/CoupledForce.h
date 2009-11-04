#ifndef COUPLEDFORCE_H
#define COUPLEDFORCE_H

#include "Kernel.h"

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
               std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
 
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _v_var;
  std::vector<Real> & _v;
};
#endif //COUPLEDFORCE_H
