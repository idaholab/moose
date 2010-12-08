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

  CHInterface(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  
  std::string _kappa_name;
  std::string _mob_name;
  std::string _grad_mob_name;
  
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;
  
};
#endif //CHInterface_H
