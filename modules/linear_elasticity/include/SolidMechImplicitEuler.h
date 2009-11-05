#ifndef SOLIDMECHIMPLICITEULER_H
#define SOLIDMECHIMPLICITEULER_H

#include "SecondDerivativeImplicitEuler.h"
#include "Material.h"


//Forward Declarations
class SolidMechImplicitEuler;

class SolidMechImplicitEuler : public SecondDerivativeImplicitEuler
{
public:

  SolidMechImplicitEuler(std::string name,
                         Parameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to=std::vector<std::string>(0),
                         std::vector<std::string> coupled_as=std::vector<std::string>(0));

  virtual void subdomainSetup();
 
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  std::vector<Real> * _density;
};
#endif //SOLIDMECHIMPLICITEULER_H
