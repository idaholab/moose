/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BondStatusPDAux.h"

template<>
InputParameters validParams<BondStatusPDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("bond_status","Auxiliary variable contains bond status");
  return params;
}

BondStatusPDAux::BondStatusPDAux(const InputParameters & parameters) :
  AuxKernel(parameters),
  _bond_critical_strain(getMaterialProperty<Real>("bond_critical_strain")),
  _bond_mechanic_strain(getMaterialProperty<Real>("bond_mechanic_strain")),
  _bond_status_old(coupledValueOld("bond_status"))
{
}

Real
BondStatusPDAux::computeValue()
{
  if (std::abs(_bond_status_old[0] - 1.0) < 0.01 && std::abs(_bond_mechanic_strain[0]) < _bond_critical_strain[0])
    return 1.0;
  else
    return 0.0;
}
