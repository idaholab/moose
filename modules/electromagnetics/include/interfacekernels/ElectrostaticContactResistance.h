#pragma once

#include "InterfaceKernel.h"

class ElectrostaticContactResistance : public InterfaceKernel
{
public:
  static InputParameters validParams();

  ElectrostaticContactResistance(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const MaterialProperty<Real> & _conductivity_master;
  const MaterialProperty<Real> & _conductivity_neighbor;
  const Real & _contact_resistance;
};
