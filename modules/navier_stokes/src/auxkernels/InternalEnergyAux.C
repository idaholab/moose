//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalEnergyAux.h"
#include "SinglePhaseFluidProperties.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", InternalEnergyAux);

InputParameters
InternalEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("density", "Density (conserved form)");
  params.addRequiredCoupledVar(NS::pressure, "Pressure");
  params.addRequiredParam<UserObjectName>("fp", "The name of the equation of state user object");
  params.addClassDescription("This AuxKernel computes the internal energy based on the equation "
                             "of state / fluid properties and the local pressure and density.");

  return params;
}

InternalEnergyAux::InternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure(coupledValue(NS::pressure)),
    _rho(coupledValue("density")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
InternalEnergyAux::computeValue()
{
  return _fp.e_from_p_rho(_pressure[_qp], _rho[_qp]);
}
