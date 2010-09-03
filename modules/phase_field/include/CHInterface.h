#ifndef CHInterface_H
#define CHInterface_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class CHInterface;

template<>
InputParameters validParams<CHInterface>();

class CHInterface : public Kernel
{
public:

  CHInterface(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;
  
};
#endif //CHInterface_H
