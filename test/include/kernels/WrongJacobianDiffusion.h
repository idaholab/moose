#ifndef WRONGJACOBIANDIFFUSION_H
#define WRONGJACOBIANDIFFUSION_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class WrongJacobianDiffusion;

template<>
InputParameters validParams<WrongJacobianDiffusion>();

/**
 * Kernel that allows to construct wrong jacobians, by multiplying a diffusion
 * kernel jacobian and/or residual with an arbitrary prefactor
 */
class WrongJacobianDiffusion : public Kernel
{
public:
  WrongJacobianDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// prefactor of the Residual
  Real _rfactor;

  /// prefactor of teh Jacobian
  Real _jfactor;
};

#endif //WRONGJACOBIANDIFFUSION_H
