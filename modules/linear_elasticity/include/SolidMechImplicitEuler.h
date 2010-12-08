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

  SolidMechImplicitEuler(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  MaterialProperty<Real> & _density;
};
#endif //SOLIDMECHIMPLICITEULER_H
