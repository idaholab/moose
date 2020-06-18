#pragma once

#include "ADInterfaceKernel.h"

class ElectrostaticContactResistance : public ADInterfaceKernel
{
public:
  static InputParameters validParams();

  ElectrostaticContactResistance(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  const MaterialProperty<Real> & _conductivity_master;
  const MaterialProperty<Real> & _conductivity_neighbor;
  const Real & _contact_resistance;
};
