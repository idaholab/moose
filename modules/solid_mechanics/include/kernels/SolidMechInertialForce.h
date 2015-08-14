/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

  SolidMechInertialForce(const InputParameters & parameters);
  SolidMechInertialForce(const std::string & deprecated_name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  const MaterialProperty<Real> & _density;
  const VariableValue & _u_old;
  const VariableValue & _vel_old;
  const VariableValue & _accel_old;
  const Real _beta;
  const Real _gamma;
  const Real _eta;

};
#endif //SOLIDMECHINERTIALFORCE_H
