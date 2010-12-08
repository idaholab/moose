#ifndef DGCONVECTION_H
#define DGCONVECTION_H

#include "DGKernel.h"

//Forward Declarations
class DGConvection;

template<>
InputParameters validParams<DGConvection>();

/**
 * DG kernel for convection
 *
 * General DG kernel that this class can handle is:
 * velocity * n_e * u_up * [v]
 *
 * [a] = [ a_1 - a_2 ]
 * u_up = u|E_e_1 if velocity.n_e >= 0
 *        u|E_e_2 if velocity.n_e < 0
 *        with n_e pointing from E_e_1 to E_e_2
 *
 */
class DGConvection : public DGKernel
{
public:
  DGConvection(const std::string & name, InputParameters parameters) ;

protected:
  virtual Real computeQpResidual(DGResidualType type);
  virtual Real computeQpJacobian(DGJacobianType type);

private:
  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
};

#endif
