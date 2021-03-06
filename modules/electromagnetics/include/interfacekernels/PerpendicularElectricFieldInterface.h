#pragma once

#include "InterfaceKernel.h"

class PerpendicularElectricFieldInterface : public VectorInterfaceKernel
{
public:
  static InputParameters validParams();

  PerpendicularElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _free_charge;
  Real _primary_eps;
  Real _secondary_eps;

  RealVectorValue _u_perp;
  RealVectorValue _secondary_perp;

  RealVectorValue _phi_u_perp;
  RealVectorValue _phi_secondary_perp;
};
