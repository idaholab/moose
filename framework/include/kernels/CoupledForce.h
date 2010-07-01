#ifndef COUPLEDFORCE_H
#define COUPLEDFORCE_H

#include "Kernel.h"

// Forward Declaration
class CoupledForce;

template<>
InputParameters validParams<CoupledForce>();

/**
 * Simple class to demonstrate off diagonal Jacobian contributijons.
 */
class CoupledForce : public Kernel
{
public:

  CoupledForce(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _v_var;
  VariableValue & _v;
};
#endif //COUPLEDFORCE_H
