//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecificInternalEnergyAux.h"

registerMooseObject("NavierStokesApp", SpecificInternalEnergyAux);

InputParameters
SpecificInternalEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("rho_u", "Momentum x-component");
  params.addCoupledVar("rho_v", 0, "Momentum y-component");
  params.addCoupledVar("rho_w", 0, "Momentum z-component");
  params.addRequiredCoupledVar("rho_et", "Total energy");
  params.addClassDescription("This AuxKernel computes the specific internal energy based "
                             "from the total and the kinetic energy.");

  return params;
}

SpecificInternalEnergyAux::SpecificInternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rho_u(coupledValue("rho_u")),
    _rho_v(coupledValue("rho_v")),
    _rho_w(coupledValue("rho_w")),
    _rho_et(coupledValue("rho_et"))
{
}

Real
SpecificInternalEnergyAux::computeValue()
{
  RealVectorValue rhou_vec(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  return (_rho_et[_qp] - 0.5 * rhou_vec * rhou_vec / _rho[_qp]) / _rho[_qp];
}
