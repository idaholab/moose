#ifndef SOLIDMECHIMPLICITEULERRZ_H
#define SOLIDMECHIMPLICITEULERRZ_H

#include "SolidMechImplicitEuler.h"

//Forward Declarations
class SolidMechImplicitEulerRZ;

template<>
InputParameters validParams<SolidMechImplicitEulerRZ>();

class SolidMechImplicitEulerRZ : public SolidMechImplicitEuler
{
public:

  SolidMechImplicitEulerRZ(const std::string & name, InputParameters parameters);

  virtual ~SolidMechImplicitEulerRZ() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};

#endif
