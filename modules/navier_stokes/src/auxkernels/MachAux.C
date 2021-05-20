//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "MachAux.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", MachAux);

namespace nms = NS;

using MetaPhysicL::raw_value;

InputParameters
MachAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>(nms::fluid, "fluid userobject");
  params.addClassDescription(
      "Mach number from fluid properties user object and material properties");
  return params;
}

MachAux::MachAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _speed(getADMaterialProperty<Real>(nms::speed)),
    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _temperature(getADMaterialProperty<Real>(nms::T_fluid)),
    _fluid(getUserObject<SinglePhaseFluidProperties>(nms::fluid))
{
}

Real
MachAux::computeValue()
{
  return raw_value(_speed[_qp] / _fluid.c_from_p_T(_pressure[_qp], _temperature[_qp]));
}
