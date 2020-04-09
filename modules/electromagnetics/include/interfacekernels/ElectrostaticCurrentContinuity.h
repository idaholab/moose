#pragma once

#include "InterfaceKernel.h"

class ElectrostaticCurrentContinuity : public InterfaceKernel
{
public:
  static InputParameters validParams();

  ElectrostaticCurrentContinuity(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const MaterialProperty<Real> & _conductivity_master;
  const MaterialProperty<Real> & _conductivity_neighbor;
};
