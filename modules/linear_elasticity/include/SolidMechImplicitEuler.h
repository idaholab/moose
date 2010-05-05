#ifndef SOLIDMECHIMPLICITEULER_H
#define SOLIDMECHIMPLICITEULER_H

#include "SecondDerivativeImplicitEuler.h"
#include "Material.h"

//Forward Declarations
class SolidMechImplicitEuler;

template<>
InputParameters validParams<SolidMechImplicitEuler>();

class SolidMechImplicitEuler : public SecondDerivativeImplicitEuler
{
public:

  SolidMechImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual void subdomainSetup();
 
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  std::vector<Real> * _density;
};
#endif //SOLIDMECHIMPLICITEULER_H
