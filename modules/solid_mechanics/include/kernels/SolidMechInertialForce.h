#ifndef SOLIDMECHINERTIALFORCE_H
#define SOLIDMECHINERTIALFORCE_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class SolidMechInertialForce;

template<>
InputParameters validParams<SolidMechInertialForce>();

class SolidMechInertialForce : public Kernel
{
public:

  SolidMechInertialForce(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  MaterialProperty<Real> & _density;
  const VariableValue & _u_old;
  const VariableValue & _vel_old;
  const VariableValue & _accel_old;
  const Real _beta;
  const Real _gamma;

};
#endif //SOLIDMECHINERTIALFORCE_H
