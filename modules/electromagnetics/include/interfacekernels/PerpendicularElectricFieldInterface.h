#pragma once

#include "InterfaceKernel.h"

class PerpendicularElectricFieldInterface;

template <>
InputParameters
validParams<PerpendicularElectricFieldInterface>();

/**
 *
 */
class PerpendicularElectricFieldInterface : public VectorInterfaceKernel
{
public:
  PerpendicularElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _free_charge;
  Real _master_eps;
  Real _neighbor_eps;

  RealVectorValue _u_perp;
  RealVectorValue _neighbor_perp;

  RealVectorValue _phi_u_perp;
  RealVectorValue _phi_neighbor_perp;
};
